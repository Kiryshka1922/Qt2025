TEMPLATE = app
TARGET = lab22_ssl_server

QT += widgets network
CONFIG += c++17

INCLUDEPATH += A:/msys64/mingw64/include
INCLUDEPATH += src/core src/network/server src/network/http

LIBS += -LA:/msys64/mingw64/lib
LIBS += -ljpeg

SOURCES += \
    src/ui/server/ssl_server_main.cpp \
    src/network/server/ssl_image_server.cpp \
    src/network/http/get_request_handler.cpp \
    src/core/jpeghandler.cpp

HEADERS += \
    src/network/server/ssl_image_server.h \
    src/network/http/http_request_handler.h \
    src/network/http/get_request_handler.h \
    src/core/jpeghandler.h \
    src/core/image_handler.h
