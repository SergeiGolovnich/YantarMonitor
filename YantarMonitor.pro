#-------------------------------------------------
#
# Project created by QtCreator 2018-08-08T10:03:02
#
#-------------------------------------------------

QT       += core gui serialport network sql testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = YantarMonitor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    widgetmonitor.cpp \
    listofmonitors.cpp \
    connectionsettings.cpp \
    monitorhistory.cpp \
    connectionmanager.cpp \
    cmdholder.cpp \
    registervaluechange.cpp \
    testtcpyantarmonitor.cpp \
    graphic.cpp \
    registerblockhelper.cpp \
    responselogger.cpp \
    dbconnectionsettings.cpp \
    dbviewer.cpp \
    test_cmdholder.cpp

HEADERS  += mainwindow.h \
    widgetmonitor.h \
    listofmonitors.h \
    connectionsettings.h \
    monitorhistory.h \
    connectionmanager.h \
    cmdholder.h \
    registervaluechange.h \
    testtcpyantarmonitor.h \
    graphic.h \
    registerblockhelper.h \
    responselogger.h \
    dbconnectionsettings.h \
    dbviewer.h \
    test_cmdholder.h
#подключение библиотеки
#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-YantarMonitorAlertAction-Desktop_Qt_5_5_1_GCC_32bit-Release/ -lYantarMonitorAlertAction
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-YantarMonitorAlertAction-Desktop_Qt_5_5_1_GCC_32bit-Debug/ -lYantarMonitorAlertAction
#else:unix: LIBS += -L$$PWD/../build-YantarMonitorAlertAction-Desktop_Qt_5_5_1_GCC_32bit-Release/ -lYantarMonitorAlertAction
#
#INCLUDEPATH += $$PWD/../YantarMonitorAlertAction
#DEPENDPATH += $$PWD/../YantarMonitorAlertAction
#
