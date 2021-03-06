##########################################################################
#
# Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# Contact: Nokia Corporation (qt-info@nokia.com)
#
# This file is part of the Qt Mobility Components.
#
# $QT_BEGIN_LICENSE:LGPL$
# GNU Lesser General Public License Usage
# This file may be used under the terms of the GNU Lesser General Public
# License version 2.1 as published by the Free Software Foundation and
# appearing in the file LICENSE.LGPL included in the packaging of this
# file. Please review the following information to ensure the GNU Lesser
# General Public License version 2.1 requirements will be met:
# http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
#
# In addition, as a special exception, Nokia gives you certain additional
# rights. These rights are described in the Nokia Qt LGPL Exception
# version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
#
# GNU General Public License Usage
# Alternatively, this file may be used under the terms of the GNU General
# Public License version 3.0 as published by the Free Software Foundation
# and appearing in the file LICENSE.GPL included in the packaging of this
# file. Please review the following information to ensure the GNU General
# Public License version 3.0 requirements will be met:
# http://www.gnu.org/copyleft/gpl.html.
#
# Other Usage
# Alternatively, this file may be used in accordance with the terms and
# conditions contained in a signed written agreement between you and Nokia.
#
#
#
#
#
# $QT_END_LICENSE$
#
##########################################################################

include(../../../features/utils.pri)

TEMPLATE = lib
CONFIG += plugin
QT += core
TARGET = $$mobilityPluginTarget(qtlandmarks_symbian)
PLUGIN_TYPE=landmarks
CONFIG += mobility
MOBILITY = location
include(../../../common.pri)

include(symbian_landmarks_defines.pri)

symbian {

        load(data_caging_paths)
        INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE
        TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = ALL -TCB
    TARGET.UID3 = $$mobilityUID(0x20031E8F)

    LIBS += \
        -lflogger \
        -leposlandmarks \
        -leposlmmultidbsearch \
        -leposlmsearchlib \
        -leposlmdbmanlib \
        -llbs \
        -leuser \
        -lefsrv \
        -lcone \
        -lbafl \
                -lapgrfx \
                -lefsrv \
                -lapmime

    pluginDep.sources = $${TARGET}.dll
    pluginDep.path = $${QT_PLUGINS_BASE_DIR}/$${PLUGIN_TYPE}
    DEPLOYMENT += pluginDep
}

HEADERS += 	inc/qlandmarkmanagerenginefactory_symbian.h \
                        inc/qlandmarkmanagerengine_symbian.h \
                        inc/qlandmarkmanagerengine_symbian_p.h \
                        inc/qlandmarkutility.h \
                        inc/qlandmarkdbeventhandler.h \
                        inc/qlandmarkrequesthandler.h

SOURCES += 	src/qlandmarkmanagerenginefactory_symbian.cpp \
                        src/qlandmarkmanagerengine_symbian.cpp \
                        src/qlandmarkmanagerengine_symbian_p.cpp \
                        src/qlandmarkutility.cpp \
                        src/qlandmarkdbeventhandler.cpp \
                        src/qlandmarkrequesthandler.cpp

target.path=$$QT_MOBILITY_PREFIX/plugins/landmarks
INSTALLS += target

