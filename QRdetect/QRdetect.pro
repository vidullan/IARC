TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    stitchimg.cpp \
    ../../harriscorner/src/Modules/HarrisCorner/klargestheap.cpp \
    extest.cpp \
    photo_process.cpp

HEADERS += \
    stitchimg.h \
    ../../harriscorner/src/Modules/HarrisCorner/klargestheap.h

INCLUDEPATH += ../../harriscorner/src/Modules/HarrisCorner/

LIBS += `pkg-config opencv --libs` -L/usr/local/lib -lzbar
