#include <QtWidgets/QApplication>
#include <QQmlContext>
#include <QQuickView>

#include "client.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QQuickView view;
  Client client;

  view.rootContext()->setContextProperty("client", &client);
  view.setSource(QUrl("qrc:/MiniTRX-client.qml"));
  view.show();

  return app.exec();
}
