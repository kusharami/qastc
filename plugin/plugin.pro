VERSION = 1.0.1

TARGET = qastc

TEMPLATE = lib
CONFIG += plugin

CONFIG += c++11 warn_off

unix|win32-g++ {
    QMAKE_CXXFLAGS_WARN_OFF -= -w
    QMAKE_CXXFLAGS += -Wall
}

win32-msvc* {
    QMAKE_CXXFLAGS_WARN_OFF -= -W0
    QMAKE_CXXFLAGS += -W3
    QMAKE_LFLAGS += /NODEFAULTLIB:LIBCMT
}

DESTDIR = $$[QT_INSTALL_PLUGINS]/imageformats

HEADERS += \
    QASTCHandler.h \
    QASTCPlugin.h \
../lib/ASTC/cASTC.h \
../lib/ASTC/ARM/astc_codec_internals.h \
../lib/ASTC/ARM/mathlib.h \
../lib/ASTC/ARM/softfloat.h \
../lib/ASTC/ARM/vectypes.h \
../lib/ASTC/ASTC_Decode.h \
../lib/ASTC/ASTC_Definitions.h \
../lib/ASTC/ASTC_Encode.h \
../lib/ASTC/ASTC_Encode_Kernel.h \
../lib/ASTC/ASTC_Host.h \
../lib/ASTC/Codec_ASTC.h \
../lib/Buffer/CodecBuffer.h \
../lib/Buffer/CodecBuffer_Block.h \
../lib/Buffer/CodecBuffer_RGBA8888.h \
../lib/Codec.h \
../lib/CommonTypes.h \
../lib/MathMacros.h

SOURCES += \
    QASTCHandler.cpp \
    QASTCPlugin.cpp \
../lib/ASTC/ARM/mathlib.cpp \
../lib/ASTC/ARM/softfloat.cpp \
../lib/ASTC/ASTC_Decode.cpp \
../lib/ASTC/ASTC_Encode.cpp \
../lib/ASTC/ASTC_Encode_Kernel.cpp \
../lib/ASTC/ASTC_Host.cpp \
../lib/ASTC/Codec_ASTC.cpp \
../lib/Buffer/CodecBuffer.cpp \
../lib/Buffer/CodecBuffer_Block.cpp \
../lib/Buffer/CodecBuffer_RGBA8888.cpp \
../lib/Codec.cpp

win32 {
    CONFIG(debug, debug|release) {
        TARGET_EXT = d.dll
    } else {
        TARGET_EXT = .dll
    }
}

OTHER_FILES += astc.json

INCLUDEPATH = ../lib ../lib/Buffer

PLUGIN_TYPE = imageformats
PLUGIN_CLASS_NAME = QASTCPlugin

DISTFILES += \
    CHANGELOG
