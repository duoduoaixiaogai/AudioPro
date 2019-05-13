#ifndef AMPLIFYFORM_H
#define AMPLIFYFORM_H

#include <QWidget>

namespace Ui {
class AmplifyForm;
}

class AmplifyForm : public QWidget
{
    Q_OBJECT

public:
    explicit AmplifyForm(QWidget *parent = nullptr);
    ~AmplifyForm();

private:
    Ui::AmplifyForm *ui;
};

#endif // AMPLIFYFORM_H
