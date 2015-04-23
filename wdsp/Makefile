TARGET   = libwdsp.a
OBJECTS  = amd.o ammod.o amsq.o analyzer.o anf.o anr.o bandpass.o calcc.o \
  cblock.o cfir.o channel.o compress.o delay.o div.o eer.o emnr.o emph.o eq.o \
  fcurve.o fir.o fmd.o fmmod.o fmsq.o gain.o gen.o iir.o iobuffs.o iqc.o \
  linux_port.o main.o meter.o meterlog10.o nob.o nobII.o osctrl.o patchpanel.o \
  resample.o RXA.o sender.o shift.o siphon.o slew.o TXA.o utilities.o wcpAGC.o
INCLUDES = -I. -I/opt/fftw/fftw-3.2.2-armhf/include
CFLAGS   = -O3 -march=armv7-a -mcpu=cortex-a9 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -ffast-math -Wall
ARFLAGS  = cru
################################################################################
CC       = arm-linux-gnueabihf-gcc
AR       = arm-linux-gnueabihf-ar
RANLIB   = arm-linux-gnueabihf-ranlib
RM       = rm -f
################################################################################
all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo ">> Linking $@"
	@$(AR) $(ARFLAGS) $@ $^
	@$(RANLIB) $@

%.o: %.c
	@echo ">> Compiling $<"
	@$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	@$(RM) $(OBJECTS)
	@$(RM) $(TARGET)
