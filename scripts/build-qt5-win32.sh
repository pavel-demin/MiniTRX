
tar -zxf qt-everywhere-opensource-src-5.4.1.tar.gz

cp qt-win32.pro qt-everywhere-opensource-src-5.4.1/qt.pro

cd qt-everywhere-opensource-src-5.4.1

./configure -release -static -largefile -opensource -confirm-license -prefix /opt/qt/qt-5.4.1-win32 -xplatform win32-g++ -device-option CROSS_COMPILE=i686-w64-mingw32- -qt-pcre -qt-zlib -no-cups -no-nis -no-dbus -no-xcb -no-linuxfb -no-fontconfig -no-evdev -no-iconv -no-gif -no-libpng -no-libjpeg -no-compile-examples -no-icu -no-eglfs -no-sql-odbc -no-sql-sqlite -no-sql-sqlite2 -no-freetype -no-harfbuzz -no-openssl -no-xkbcommon -no-pulseaudio -no-alsa -no-gtkstyle -no-qml-debug -skip qtquick1 -skip qtdeclarative -nomake examples -nomake tests

make
sudo make install
