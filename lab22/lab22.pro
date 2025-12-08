TEMPLATE = app
TARGET = lab22

QT += widgets network
CONFIG += c++17

INCLUDEPATH += A:/msys64/mingw64/include

LIBS += -LA:/msys64/mingw64/lib
LIBS += -ljpeg

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    jpeghandler.cpp \
    image_handler_factory.cpp \
    image_manager.cpp \
    libjpeg_save_strategy.cpp \
    ui_state_observer.cpp

HEADERS += \
    mainwindow.h \
    common_types.h \
    jpeghandler.h \
    image_handler.h \
    image_handler_factory.h \
    image_manager.h \
    save_strategy.h \
    libjpeg_save_strategy.h \
    image_state_observer.h \
    ui_state_observer.h

