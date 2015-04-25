wget http://www.mega-nerd.com/SRC/libsamplerate-0.1.8.tar.gz
tar -zxf libsamplerate-0.1.8.tar.gz
cd libsamplerate-0.1.8
./configure --host=arm-linux-gnueabihf --prefix=/opt/libsamplerate/libsamplerate-0.1.8-armhf --disable-fftw CFLAGS="-O3 -mcpu=cortex-a9 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -ffast-math"
make
sudo make install
