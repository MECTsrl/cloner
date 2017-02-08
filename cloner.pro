#-------------------------------------------------
#
# Project created by QtCreator 2015-01-22T18:12:14
#
#-------------------------------------------------

TEMPLATE = app

TARGET = cloner

QT += \
    core \
    gui \

INCLUDEPATH += \
    $$(DEV_IMAGE)/usr/include \

LIBS += \
    -L$$(DEV_IMAGE)/usr/lib \
    -lATCMinputdialog \
    -lts \

### LIBS = -L/opt/Trolltech/lib -Wl,-Bstatic -lts -lATCMinputdialog -L/opt/Trolltech/lib -lQtGui -lQtNetwork -lQtCore -Wl,-Bdynamic -lpthread

DEFINES += SVN_REV=6.4
#DEFINES += SVN_REV=\"$(VERSION)\"
#DEFINES += 'SVN_REV="rev. $(shell svnversion -n .)"'

SOURCES += \
    main.cpp \
    cloner.cpp \

HEADERS += \
    cloner.h \

FORMS += \
    cloner.ui \

RESOURCES += \
    resources.qrc \
