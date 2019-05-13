#ifndef AMPLIFYDIALOG_H
#define AMPLIFYDIALOG_H

#include <QDialog>

namespace Ui {
  class AmplifyDialog;
}

class AmplifyDialog : public QDialog
{
  Q_OBJECT

public:
  explicit AmplifyDialog(QWidget *parent = nullptr);
  ~AmplifyDialog();

private:
  Ui::AmplifyDialog *ui;
};

#endif // AMPLIFYDIALOG_H
