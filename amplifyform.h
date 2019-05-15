#ifndef AMPLIFYFORM_H
#define AMPLIFYFORM_H

#include "ui_amplifyform.h"
#include "amplify.h"

#include <QWidget>
#include <QDoubleValidator>

namespace Ui {
    class AmplifyForm;
}

namespace Renfeng {
    class AmplifyForm : public QWidget
    {
        Q_OBJECT
      friend class EffectAmplify;

    public:
        explicit AmplifyForm(QWidget *parent = nullptr);
        ~AmplifyForm();

       void setAmpValue(double value);
        void setAmpRange(double min, double max);
        void setSliderValue(int value);
        void setSliderRange(int min, int max);
        void setPeakValue(double value);
        void setPeakRange(double min, double max);
        void setClipSel(bool sel = false);
        void setClipEnable(bool enable = false);
        bool getClipSel();

    private:
        Ui::AmplifyForm *ui;
    };
}
#endif // AMPLIFYFORM_H
