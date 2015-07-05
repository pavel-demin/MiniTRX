qt_ver=5.5.0
qt_dir=qt-everywhere-opensource-src-${qt_ver}
qt_tar=${qt_dir}.tar.gz
qt_url=http://download.qt.io/official_releases/qt/5.5/${qt_ver}/single/${qt_tar}

test -f ${qt_tar} || curl -L ${qt_url} -o ${qt_tar}

tar -zxf ${qt_tar}

cp patches/qt-win32.pro ${qt_dir}/qt.pro

sed -i 's:"qplatformpixmap.h":"QtGui/qpa/qplatformpixmap.h":' ${qt_dir}/qtbase/src/gui/painting/qbrush.cpp

cd ${qt_dir}

./configure -release -static -largefile -opensource -confirm-license -prefix /opt/qt/qt-${qt_ver}-win32 -xplatform win32-g++ -device-option CROSS_COMPILE=i686-w64-mingw32- -qt-pcre -qt-zlib -no-cups -no-nis -no-dbus -no-xcb -no-linuxfb -no-fontconfig -no-evdev -no-iconv -no-gif -no-libpng -no-libjpeg -no-compile-examples -no-icu -no-eglfs -no-sql-odbc -no-sql-sqlite -no-sql-sqlite2 -no-freetype -no-harfbuzz -no-openssl -no-xkbcommon -no-pulseaudio -no-alsa -no-gtkstyle -no-qml-debug -skip qtcanvas3d -nomake examples -nomake tests

make -j 4
sudo make install
