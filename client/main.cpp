#include <QtWidgets/QApplication>
#include <QtUiTools>

#include "client.h"

int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(client);
  QApplication app(argc, argv);
  Client client;
  client.show();
  return app.exec();
}
