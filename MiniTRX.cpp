#include <stdint.h>

#include <iostream>

#include <QtGui>
#include <QtUiTools/QUiLoader>
#include <QtCore/QFile>
#include <QtWidgets/QVBoxLayout>

#include "MiniTRX.h"

using namespace std;

//------------------------------------------------------------------------------

MiniTRX::MiniTRX(QWidget *parent) :
  QWidget(parent), fSpectrum(0)
{
  QUiLoader loader;

  QFile file("MiniTRX.ui");
  file.open(QFile::ReadOnly);
  QWidget *formWidget = loader.load(&file, this);
  file.close();

  fSpectrum = findChild<QWidget *>("spectrum");

  QMetaObject::connectSlotsByName(this);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(formWidget);
  setLayout(layout);

  setFixedSize(layout->maximumSize());

}

//------------------------------------------------------------------------------

MiniTRX::~MiniTRX()
{
}

//------------------------------------------------------------------------------

/*
void MiniTRX::on_Quit_clicked()
{
  qApp->quit();
}

*/
