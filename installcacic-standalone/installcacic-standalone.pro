#-------------------------------------------------
#
# Project Cacic Agente, module cacic-service, created by Lightbase
#
# Developers: Eric Menezes Noronha (eric.m.noronha@lightbase.com.br); GitHub: ericmenezes
#             Thiago Rocha         (thiago.rocha@lightbase.com.br)  ;
#
#-------------------------------------------------

##############################################################################
#SEMPRE MUDAR A VERSÃO DE BUILD (o terceiro número) AO REALIZAR QUALQUER BUILD.#
VERSION_MAJOR = 3
VERSION_MINOR = 2
VERSION_BUILD = 0
DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
           "VERSION_MINOR=$$VERSION_MINOR"\
           "VERSION_BUILD=$$VERSION_BUILD"
VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}
##############################################################################
TARGET = install-cacic-sa

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lws2_32
LIBS += -lshlwapi
LIBS += -liphlpapi

TEMPLATE = app

CONFIG += static

static {
    CONFIG += static
    CONFIG += release
    DEFINES += STATIC #Equivalent to "#define STATIC" in source code
    message("Static build.")
}

SOURCES += main.cpp \
    installcacicsa.cpp \
    commsa.cpp \
    ../src/servicecontroller.cpp \
    ../src/simplejson/src/JSON.cpp \
    ../src/simplejson/src/JSONValue.cpp \
    ../src/wcomputer.cpp

HEADERS += testeinstallcacic.h \
    installcacicsa.h \
    commsa.h \
    ../src/servicecontroller.h \
    ../src/simplejson/src/JSON.h \
    ../src/simplejson/src/JSONValue.h \
    ../src/tinydir.h \
    ../src/wcomputer.cpp

INCLUDEPATH += ../src

#include(deployment.pri)
#qtcAddDeployment()

