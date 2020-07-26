######################################################################
# Automatically generated by qmake (3.1) Sat Jul 25 09:28:06 2020
######################################################################

QT -= gui
TEMPLATE = lib
TARGET = gme
VERSION = 0.6.3
INCLUDEPATH += . gme

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += LIBGME_VISIBILITY
DEFINES += BLARGG_BUILD_DLL
DEFINES += GME_DISABLE_STEREO_DEPTH
DEFINES += USE_GME_NSF
DEFINES += USE_GME_NSFE

# Input
HEADERS += gme/blargg_common.h \
           gme/blargg_config.h \
           gme/blargg_endian.h \
           gme/blargg_source.h \
           gme/Blip_Buffer.h \
           gme/Classic_Emu.h \
           gme/Data_Reader.h \
           gme/gme.h \
           gme/Gme_File.h \
           gme/M3u_Playlist.h \
           gme/Multi_Buffer.h \
           gme/Music_Emu.h \
           gme/Nes_Apu.h \
           gme/Nes_Cpu.h \
           gme/nes_cpu_io.h \
           gme/Nes_Fme7_Apu.h \
           gme/Nes_Namco_Apu.h \
           gme/Nes_Oscs.h \
           gme/Nes_Vrc6_Apu.h \
           gme/Nsf_Emu.h \
           gme/Nsfe_Emu.h
SOURCES += gme/Blip_Buffer.cpp \
           gme/Classic_Emu.cpp \
           gme/Data_Reader.cpp \
           gme/gme.cpp \
           gme/Gme_File.cpp \
           gme/M3u_Playlist.cpp \
           gme/Multi_Buffer.cpp \
           gme/Music_Emu.cpp \
           gme/Nes_Apu.cpp \
           gme/Nes_Cpu.cpp \
           gme/Nes_Fme7_Apu.cpp \
           gme/Nes_Namco_Apu.cpp \
           gme/Nes_Oscs.cpp \
           gme/Nes_Vrc6_Apu.cpp \
           gme/Nsf_Emu.cpp \
           gme/Nsfe_Emu.cpp
