TEMPLATE=app
INCLUDEPATH += ../../location

QT += webkit network

HEADERS = mapwindow.h
SOURCES = mapwindow.cpp \
          main.cpp

include(../examples.pri)
qtAddLibrary(QtLocation)

logfile.path = $$DESTDIR
logfile.files = nmealog.txt
logfile.CONFIG = no_link no_dependencies explicit_dependencies no_build combine ignore_no_exist no_clean
INSTALLS += logfile
build_pass:ALL_DEPS+=install_logfile