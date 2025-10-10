QT += core gui widgets
TARGET = image_filter_app
TEMPLATE = app
SOURCES += main.cpp Widget.cpp ImageInfoWidget.cpp filter2D.cpp RGBToHalf.cpp binarizationFunction.cpp
HEADERS += Widget.h ImageInfoWidget.h RGBToHalf.h binarizationFunction.h
CONFIG += rerunqmake