#include "amplifyform.h"
#include "ui_amplifyform.h"

AmplifyForm::AmplifyForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AmplifyForm)
{
    ui->setupUi(this);
}

AmplifyForm::~AmplifyForm()
{
    delete ui;
}
