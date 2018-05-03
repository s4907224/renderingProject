######################################################################
# Automatically generated by qmake (3.0) Thu Jan 26 16:35:29 2017
######################################################################

include(common.pri)
#include(/home/s4907224/Realtime_Rendering/rendering_workshops/common/common.pri)
TARGET = sandisk

OBJECTS_DIR = obj

MOC_DIR = moc

INCLUDEPATH +=./include

DESTDIR=./

# Input
HEADERS += include/sandiskscene.h \
           include/camera.h \
           include/fixedcamera.h \
           include/scene.h \
           include/trackballcamera.h

SOURCES += src/main.cpp src/sandiskscene.cpp \
           src/camera.cpp \
           src/fixedcamera.cpp \
           src/scene.cpp \
           src/trackballcamera.cpp

OTHER_FILES +=  shaders/env_vert.glsl \
                shaders/env_frag.glsl \
                shaders/beckmann_vert.glsl \
                shaders/beckmann_frag.glsl \
                shaders/shadow_vert.glsl \
                shaders/shadow_frag.glsl \

DISTFILES += $OTHER_FILES


