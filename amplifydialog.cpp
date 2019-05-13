#include "amplifydialog.h"
#include "ui_amplifydialog.h"

AmplifyDialog::AmplifyDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::AmplifyDialog)
{
  ui->setupUi(this);
}

AmplifyDialog::~AmplifyDialog()
{
  delete ui;
}
