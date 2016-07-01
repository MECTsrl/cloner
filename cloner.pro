#-------------------------------------------------
#
# Project created by QtCreator 2015-01-22T18:12:14
#
#-------------------------------------------------

QT       += core gui
CONFIG += static
CONFIG += staticlib
DEFINES += STATIC
DEFINES -= QT_SHARED
TARGET = cloner
TEMPLATE = app

LIBS += \
-lts \
-lATCMinputdialog


DEFINES+=SVN_REV=6.4
#DEFINES+=SVN_REV=\"$(VERSION)\"
#DEFINES+='SVN_REV="rev. $(shell svnversion -n .)"'

SOURCES += main.cpp\
        cloner.cpp

HEADERS  += cloner.h

FORMS    += cloner.ui

RESOURCES += \
    resources.qrc
