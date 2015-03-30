#include <QtWidgets/QApplication>

#include "MiniTRX.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  MiniTRX rxtx;
  rxtx.show();
  return app.exec();
}


