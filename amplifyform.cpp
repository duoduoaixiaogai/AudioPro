#include "amplifyform.h"

namespace Renfeng {
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

    void AmplifyForm::setAmpValue(double value) {
        if (ui->lineEdit) {
                ui->lineEdit->setText(QString("%1").arg(value));
            }
    }

    void AmplifyForm::setAmpRange(double min, double max) {
        if (ui->lineEdit) {
                QDoubleValidator vldAmp;
                vldAmp.setRange(min, max);
                ui->lineEdit->setValidator(&vldAmp);
            }
    }

    void AmplifyForm::setSliderValue(int value) {
        if (ui->horizontalSlider) {
                ui->horizontalSlider->setValue(value);
            }
    }

    void AmplifyForm::setSliderRange(int min, int max) {
        if (ui->horizontalSlider) {
                ui->horizontalSlider->setRange(min, max);
            }
    }

    void AmplifyForm::setPeakValue(double value) {
        if (ui->lineEdit_2) {
                ui->lineEdit_2->setText(QString("%1").arg(value));
            }
    }

    void AmplifyForm::setPeakRange(double min, double max) {
        if (ui->lineEdit_2) {
                QDoubleValidator vldNewPeak;
                vldNewPeak.setRange(min, max);
                ui->lineEdit_2->setValidator(&vldNewPeak);
            }
    }

    void AmplifyForm::setClipSel(bool sel) {
        if (ui->checkBox) {
                ui->checkBox->setChecked(sel);
            }
    }

    void AmplifyForm::setClipEnable(bool enable) {
        if (ui->checkBox) {
                ui->checkBox->setEnabled(enable);
            }
    }

    bool AmplifyForm::getClipSel() {
      if (ui->checkBox) {
          return ui->checkBox->isChecked();
        }
    }
}
