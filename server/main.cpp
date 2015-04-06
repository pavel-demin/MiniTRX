#include <QtCore/QCoreApplication>

#include "server.h"

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  Server server(1001);
  return app.exec();
}
