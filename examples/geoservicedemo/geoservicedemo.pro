QT += network

TARGET = GeoServiceDemo
TEMPLATE = app

INCLUDEPATH += ../../src/location \
                ../../src/location/maps

SOURCES += main.cpp\
        routepresenter.cpp \
        placepresenter.cpp \
        tabbedwindow.cpp \
        routetab.cpp \
        geocodingtab.cpp \
        revgeocodingtab.cpp \
        servicestab.cpp 

HEADERS  += routepresenter.h \
        placepresenter.h \
        tabbedwindow.h \
        routetab.h \
        geocodingtab.h \
        revgeocodingtab.h \
        servicestab.h 

include(../examples.pri)

CONFIG += mobility
MOBILITY = location
contains(QT_MAJOR_VERSION, 4):lessThan(QT_MINOR_VERSION, 7){
    MOBILITY += bearer
}
    
symbian: {
     TARGET.CAPABILITY = Location NetworkServices ReadUserData WriteUserData
}