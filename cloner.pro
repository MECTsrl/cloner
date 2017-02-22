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

DEFINES += SVN_REV=\"$(VERSION)\"

SOURCES += \
    main.cpp \
    cloner.cpp \

HEADERS += \
    cloner.h \

FORMS += \
    cloner.ui \

RESOURCES += \
    resources.qrc \
