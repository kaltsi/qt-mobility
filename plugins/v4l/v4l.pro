TEMPLATE = lib
CONFIG += plugin
TARGET = $$qtLibraryTarget(v4lengine)

PLUGIN_TYPE = mediaservice

include (../../common.pri)

qtAddLibrary(QtMedia)

DEPENDPATH += .

INCLUDEPATH += ../../multimedia

HEADERS += v4lserviceplugin.h
SOURCES += v4lserviceplugin.cpp

include(radio/radio.pri)
include(camera/camera.pri)

target.path=$$QT_MOBILITY_PREFIX/plugins/mediaservice
INSTALLS+=target