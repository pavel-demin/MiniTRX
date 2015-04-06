TARGET = MiniTRX-server
QT += core websockets
QT -= gui
CONFIG += console debug_and_release
TEMPLATE = app
QMAKE_CFLAGS = -fomit-frame-pointer -fstrict-aliasing -ffast-math
INCLUDEPATH += wdsp /opt/fftw/fftw-3.2.2-armhf/include
LIBS += -L/opt/fftw/fftw-3.2.2-armhf/lib -lfftw3f
HEADERS = server.h
SOURCES = server.cpp main.cpp
SOURCES += wdsp/amd.c wdsp/ammod.c wdsp/amsq.c wdsp/anf.c wdsp/anr.c \
  wdsp/bandpass.c wdsp/cblock.c wdsp/channel.c wdsp/compress.c wdsp/delay.c \
  wdsp/div.c wdsp/eer.c wdsp/emnr.c wdsp/emph.c wdsp/fcurve.c wdsp/fir.c \
  wdsp/gain.c wdsp/gen.c wdsp/iir.c wdsp/meter.c wdsp/meterlog10.c wdsp/nob.c \
  wdsp/nobII.c wdsp/osctrl.c wdsp/patchpanel.c wdsp/resample.c wdsp/RXA.c \
  wdsp/shift.c wdsp/TXA.c wdsp/utilities.c wdsp/wcpAGC.c
