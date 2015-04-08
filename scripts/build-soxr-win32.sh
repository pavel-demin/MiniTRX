tar -lxf soxr-0.1.1-Source.tar.xz

mkdir soxr-0.1.1-Build
cd soxr-0.1.1-Build

cmake ../soxr-0.1.1-Source -DCMAKE_TOOLCHAIN_FILE=../patches/win32.cmake -DCMAKE_INSTALL_PREFIX=/opt/soxr/soxr-0.1.1-win32 -DHAVE_WORDS_BIGENDIAN_EXITCODE=1 -DBUILD_SHARED_LIBS=OFF -DWITH_OPENMP=OFF -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF

make
sudo make install
