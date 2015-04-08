wget http://download.qt.io/official_releases/qt/5.4/5.4.1/single/qt-everywhere-opensource-src-5.4.1.tar.gz

tar -zxf qt-everywhere-opensource-src-5.4.1.tar.gz

cp -a patches/linux-arm-gnueabihf-g++ qt-everywhere-opensource-src-5.4.1/qtbase/mkspecs/

cd qt-everywhere-opensource-src-5.4.1

./configure -release -static -largefile -opensource -confirm-license -prefix /opt/qt/qt-5.4.1-armhf -xplatform linux-arm-gnueabihf-g++ -qt-pcre -qt-zlib -no-cups -no-nis -no-dbus -no-xcb -no-linuxfb -no-fontconfig -no-evdev -no-iconv -no-gif -no-libpng -no-libjpeg -no-compile-examples -no-gui -no-widgets -no-opengl -no-icu -no-eglfs -no-sql-sqlite -no-sql-sqlite2 -no-freetype -no-harfbuzz -no-openssl -no-xkbcommon -no-pulseaudio -no-alsa -no-gtkstyle -no-qml-debug -skip qtquick1 -skip qtdeclarative -nomake examples -nomake tests

make module-qtwebsockets
sudo make module-qtwebsockets-install_subtargets
