#-------------------------------------------------
#
# Project created by QtCreator 2017-06-22T13:47:44
#
#-------------------------------------------------

QT       += core gui network serialport charts

LIBS += -L$$PWD/./ -lNIDAQmx

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mri
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    programmerthread.cpp \
    settings.cpp \
    utils.cpp \
    experimentdialog.cpp

HEADERS  += mainwindow.h \
    NIDAQmx.h \
    programmerthread.h \
    settings.h \
    types.h \
    utils.h \
    experimentdialog.h \
    experiment.h

FORMS    += mainwindow.ui \
    experimentdialog.ui
