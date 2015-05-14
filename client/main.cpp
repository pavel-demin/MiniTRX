/*
 *  MiniTRX: minimalist user interface for the Red Pitaya SDR transceiver
 *  Copyright (C) 2014-2015  Pavel Demin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtWidgets/QApplication>
#include <QtQuick>

#include "client.h"
#include "grid.h"
#include "spectrum.h"
#include "waterfall.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QQuickView view;
  Client client;
  QObject *rootObject = 0;
  Spectrum *spectrum = 0;
  Waterfall *waterfall = 0;

  qmlRegisterType<Grid>("MiniTRX", 1, 0, "Grid");
  qmlRegisterType<Spectrum>("MiniTRX", 1, 0, "Spectrum");
  qmlRegisterType<Waterfall>("MiniTRX", 1, 0, "Waterfall");

  view.rootContext()->setContextProperty("client", &client);
  view.setSource(QUrl("qrc:/MiniTRX-client.qml"));

  rootObject = view.rootObject();

  spectrum = rootObject->findChild<Spectrum *>("spectrum");
  client.setSpectrum(spectrum);

  waterfall = rootObject->findChild<Waterfall *>("waterfall");
  client.setWaterfall(waterfall);

  view.setMinimumSize(QSize(782, 500));
  view.setMaximumSize(QSize(782, 500));
  view.show();

  return app.exec();
}
