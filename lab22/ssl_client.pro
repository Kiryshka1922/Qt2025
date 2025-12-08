TEMPLATE = app
TARGET = lab22_ssl_client

QT += widgets network
CONFIG += c++17

INCLUDEPATH += A:/msys64/mingw64/include
INCLUDEPATH += src/core src/network/client src/network/http src/ui/observer src/ui/client

LIBS += -LA:/msys64/mingw64/lib
LIBS += -ljpeg

SOURCES += \
    src/apps/client/ssl_client_main.cpp \
    src/ui/client/ssl_client_mainwindow.cpp \
    src/core/jpeghandler.cpp \
    src/core/image_handler_factory.cpp \
    src/core/image_manager.cpp \
    src/core/libjpeg_save_strategy.cpp \
    src/ui/observer/ui_state_observer.cpp \
    src/network/client/ssl_image_client.cpp

HEADERS += \
    src/ui/client/ssl_client_mainwindow.h \
    src/core/common_types.h \
    src/core/jpeghandler.h \
    src/core/image_handler.h \
    src/core/image_handler_factory.h \
    src/core/image_manager.h \
    src/core/save_strategy.h \
    src/core/libjpeg_save_strategy.h \
    src/ui/observer/image_state_observer.h \
    src/ui/observer/ui_state_observer.h \
    src/network/client/ssl_image_client.h
