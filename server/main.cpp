#include <QtCore/QCoreApplication>

#include "server.h"

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  Server server(1001);
  QObject::connect(&server, &Server::closed, &app, &QCoreApplication::quit);
  return app.exec();
}
