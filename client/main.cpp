#include <QtWidgets/QApplication>
#include <QtQuick>

#include "client.h"
#include "spectrum.h"
#include "waterfall.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QQuickView view;
  Client client;
  QObject *rootObject = 0;
  Spectrum *spectrum = 0;
  Waterfall *waterafll = 0;

  qsrand(1);

  qmlRegisterType<Spectrum>("MiniTRX", 1, 0, "Spectrum");
  qmlRegisterType<Waterfall>("MiniTRX", 1, 0, "Waterfall");

  view.rootContext()->setContextProperty("client", &client);
  view.setSource(QUrl("qrc:/MiniTRX-client.qml"));

  rootObject = view.rootObject();

  spectrum = rootObject->findChild<Spectrum *>("spectrum");
  client.setSpectrum(spectrum);

  waterafll = rootObject->findChild<Waterfall *>("waterfall");
  client.setWaterfall(waterafll);

  view.setMinimumSize(QSize(782, 500));
  view.setMaximumSize(QSize(782, 500));
  view.show();

  return app.exec();
}
