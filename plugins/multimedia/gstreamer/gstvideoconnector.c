/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "gstvideoconnector.h"

GST_DEBUG_CATEGORY_STATIC (video_connector_debug);
#define GST_CAT_DEFAULT video_connector_debug

static GstStaticPadTemplate gst_video_connector_sink_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
                         GST_PAD_SINK,
                         GST_PAD_ALWAYS,
                         GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate gst_video_connector_src_factory =
GST_STATIC_PAD_TEMPLATE ("src",
                         GST_PAD_SRC,
                         GST_PAD_ALWAYS,
                         GST_STATIC_CAPS_ANY);

#define _do_init(bla) \
    GST_DEBUG_CATEGORY_INIT (video_connector_debug, \
    "video-connector", 0, "An identity like element for reconnecting video stream");

GST_BOILERPLATE_FULL (GstVideoConnector, gst_video_connector, GstElement,
                      GST_TYPE_ELEMENT, _do_init);

static void gst_video_connector_dispose (GObject * object);
static GstFlowReturn gst_video_connector_chain (GstPad * pad, GstBuffer * buf);
static GstFlowReturn gst_video_connector_buffer_alloc (GstPad * pad,
                                                       guint64 offset, guint size, GstCaps * caps, GstBuffer ** buf);
static GstStateChangeReturn gst_video_connector_change_state (GstElement *
                                                              element, GstStateChange transition);
static gboolean gst_video_connector_handle_sink_event (GstPad * pad,
                                                       GstEvent * event);
static gboolean gst_video_connector_new_buffer_probe(GstObject *pad, GstBuffer *buffer, guint * object);

static void
gst_video_connector_base_init (gpointer g_class)
{
    GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

    gst_element_class_set_details_simple (element_class, "Video Connector",
                                          "Generic",
                                          "An identity like element used for reconnecting video stream",
                                          "Dmytro Poplavskiy <dmytro.poplavskiy@nokia.com>");
    gst_element_class_add_pad_template (element_class,
                                        gst_static_pad_template_get (&gst_video_connector_sink_factory));
    gst_element_class_add_pad_template (element_class,
                                        gst_static_pad_template_get (&gst_video_connector_src_factory));
}

static void
gst_video_connector_class_init (GstVideoConnectorClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    gobject_class->dispose = gst_video_connector_dispose;
    gstelement_class->change_state = gst_video_connector_change_state;
}

static void
gst_video_connector_init (GstVideoConnector *element,
                          GstVideoConnectorClass *g_class)
{
    element->sinkpad =
            gst_pad_new_from_static_template (&gst_video_connector_sink_factory,
                                              "sink");
    gst_pad_set_chain_function(element->sinkpad,
                               GST_DEBUG_FUNCPTR (gst_video_connector_chain));
    gst_pad_set_event_function(element->sinkpad,
                               GST_DEBUG_FUNCPTR (gst_video_connector_handle_sink_event));
    gst_pad_set_bufferalloc_function(element->sinkpad,
                                     GST_DEBUG_FUNCPTR (gst_video_connector_buffer_alloc));

    gst_element_add_pad (GST_ELEMENT (element), element->sinkpad);

    element->srcpad =
            gst_pad_new_from_static_template (&gst_video_connector_src_factory,
                                              "src");
    gst_pad_add_buffer_probe(element->srcpad,
                             G_CALLBACK(gst_video_connector_new_buffer_probe), element);
    gst_element_add_pad (GST_ELEMENT (element), element->srcpad);

    element->relinked = FALSE;
    gst_segment_init (&element->segment, GST_FORMAT_TIME);
    element->latest_buffer = NULL;
}

static void
gst_video_connector_reset (GstVideoConnector * element)
{
    element->relinked = FALSE;
    if (element->latest_buffer != NULL) {
        gst_buffer_unref (element->latest_buffer);
        element->latest_buffer = NULL;
    }
    gst_segment_init (&element->segment, GST_FORMAT_UNDEFINED);
}

static void
gst_video_connector_dispose (GObject * object)
{
    GstVideoConnector *element = GST_VIDEO_CONNECTOR (object);

    gst_video_connector_reset (element);

    G_OBJECT_CLASS (parent_class)->dispose (object);
}


static GstFlowReturn
gst_video_connector_buffer_alloc (GstPad * pad, guint64 offset, guint size,
                                  GstCaps * caps, GstBuffer ** buf)
{
    GstVideoConnector *element;
    GstFlowReturn res = GST_FLOW_OK;
    element = GST_VIDEO_CONNECTOR (GST_PAD_PARENT (pad));

    GST_OBJECT_LOCK (element);
    gst_object_ref(element->srcpad);
    GST_OBJECT_UNLOCK (element);

    res = gst_pad_alloc_buffer(element->srcpad, offset, size, caps, buf);
    gst_object_unref (element->srcpad);

    GST_DEBUG_OBJECT (element, "buffer alloc finished: %s", gst_flow_get_name (res));

    return res;
}


static gboolean gst_video_connector_new_buffer_probe(GstObject *pad, GstBuffer *buffer, guint * object)
{
    GstVideoConnector *element = GST_VIDEO_CONNECTOR (object);

    /*
      If relinking is requested, the current buffer should be rejected and
      the new segment + previous buffer should be pushed first
    */

    if (element->relinked)
        GST_LOG_OBJECT(element, "rejected buffer because of new segment request");

    return !element->relinked;
}


static GstFlowReturn
gst_video_connector_chain (GstPad * pad, GstBuffer * buf)
{
    GstFlowReturn res;
    GstVideoConnector *element;

    element = GST_VIDEO_CONNECTOR (gst_pad_get_parent (pad));

    do {
        /*
          Resend the segment message and last buffer to preroll the new sink.
          Sinks can be changed multiple times while paused,
          while loop allows to send the segment message and preroll
          all of them with the same buffer.
        */
        while (element->relinked) {
            element->relinked = FALSE;

            gint64 pos = element->segment.last_stop;

            if (GST_BUFFER_TIMESTAMP_IS_VALID(element->latest_buffer)) {
                pos = GST_BUFFER_TIMESTAMP (element->latest_buffer);
            }

            //push a new segment and last buffer
            GstEvent *ev = gst_event_new_new_segment (TRUE,
                                                      element->segment.rate,
                                                      element->segment.format,
                                                      pos, //start
                                                      element->segment.stop,
                                                      pos);

            GST_DEBUG_OBJECT (element, "Pushing new segment event");
            if (!gst_pad_push_event (element->srcpad, ev)) {
                GST_WARNING_OBJECT (element,
                                    "Newsegment handling failed in %" GST_PTR_FORMAT,
                                    element->srcpad);
            }

            if (element->latest_buffer) {
                GST_DEBUG_OBJECT (element, "Pushing latest buffer...");
                gst_buffer_ref(element->latest_buffer);
                gst_pad_push(element->srcpad, element->latest_buffer);
            }
        }

        gst_buffer_ref(buf);

        //it's possible video sink is changed during gst_pad_push blocked by
        //pad lock, in this case ( element->relinked == TRUE )
        //the buffer should be rejected by the buffer probe and
        //the new segment + prev buffer should be sent before

        GST_LOG_OBJECT (element, "Pushing buffer...");
        res = gst_pad_push (element->srcpad, buf);
        GST_LOG_OBJECT (element, "Pushed buffer: %s", gst_flow_get_name (res));

    } while (element->relinked);


    if (element->latest_buffer) {
        gst_buffer_unref (element->latest_buffer);
        element->latest_buffer = NULL;
    }

    element->latest_buffer = gst_buffer_ref(buf);

    gst_buffer_unref(buf);
    gst_object_unref (element);

    return res;
}

static GstStateChangeReturn
gst_video_connector_change_state (GstElement * element,
                                  GstStateChange transition)
{
    GstVideoConnector *connector;
    GstStateChangeReturn result;

    connector = GST_VIDEO_CONNECTOR(element);
    result = GST_ELEMENT_CLASS (parent_class)->change_state(element, transition);

    switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
        gst_video_connector_reset (connector);
        break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
        connector->relinked = FALSE;
        break;
    default:
        break;
    }

    return result;
}

static gboolean
gst_video_connector_handle_sink_event (GstPad * pad, GstEvent * event)
{
    if (GST_EVENT_TYPE (event) == GST_EVENT_NEWSEGMENT) {
        GstVideoConnector *element = GST_VIDEO_CONNECTOR (gst_pad_get_parent (pad));

        gboolean update;
        GstFormat format;
        gdouble rate, arate;
        gint64 start, stop, time;

        gst_event_parse_new_segment_full (event, &update, &rate, &arate, &format,
                                          &start, &stop, &time);

        GST_DEBUG_OBJECT (element,
                          "NEWSEGMENT update %d, rate %lf, applied rate %lf, "
                          "format %d, " "%" G_GINT64_FORMAT " -- %" G_GINT64_FORMAT ", time %"
                          G_GINT64_FORMAT, update, rate, arate, format, start, stop, time);

        gst_segment_set_newsegment_full (&element->segment, update,
                                         rate, arate, format, start, stop, time);

        gst_object_unref (element);
    }

    return gst_pad_event_default (pad, event);
}