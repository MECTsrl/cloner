#-------------------------------------------------
#
# Project created by QtCreator 2021-03-29T08:00:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cloner
#TARGET = hmi_only


DEFINES +=  MECT_BUILD_MAJOR=\"$(MECT_BUILD_MAJOR)\" \
            MECT_BUILD_MINOR=\"$(MECT_BUILD_MINOR)\" \
            MECT_BUILD_BUILD=\"$(MECT_BUILD_BUILD)\"

#DEFINES += SVN_REV=\"$(VERSION)\"

TEMPLATE = app



INCLUDEPATH += \
    $$(DEV_IMAGE)/usr/include \

LIBS += \
    -L$$(DEV_IMAGE)/usr/lib \
    -lATCMinputdialog \
    -lts \

#INCLUDEPATH += \
#    $$(MECT_RFSDIR)/usr/include \

#LIBS += \
#    -L$$(MECT_RFSDIR)/usr/lib \
#    -lATCMinputdialog \
#    -lts \


SOURCES += main.cpp\
        maincloner.cpp \
    chooseimage.cpp \
    managevpn.cpp \
    managessh.cpp \
    timeset.cpp \
    netcfg.cpp \
    info.cpp

HEADERS  += maincloner.h \
    chooseimage.h \
    publics.h \
    managevpn.h \
    managessh.h \
    timeset.h \
    netcfg.h \
    info.h

FORMS    += maincloner.ui \
    chooseimage.ui \
    managevpn.ui \
    managessh.ui \
    timeset.ui \
    netcfg.ui \
    info.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    cloner.qss

#target.path = /local/root

#INSTALLS += target

