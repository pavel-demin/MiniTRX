wget http://www.vesperix.com/arm/fftw-arm/source/fftw-3.2.2-arm.tar.gz

tar -zxf fftw-3.2.2-arm.tar.gz
cd fftw-3.2.2-arm

./configure --prefix=/opt/fftw/fftw-3.2.2-armhf --build=x86_64-pc-linux-gnu --host=arm-linux-gnueabihf --enable-single --enable-neon --enable-perf-events --disable-fortran ARM_CPU_TYPE=cortex-a9 ARM_FLOAT_ABI=hard

make
sudo make install
