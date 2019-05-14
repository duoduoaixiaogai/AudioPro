#ifndef AMPLIFYFORM_H
#define AMPLIFYFORM_H

#include "amplify.h"

#include <QWidget>

namespace Ui {
    class AmplifyForm;
}

namespace RF {
    class AmplifyForm : public QWidget
    {
        Q_OBJECT

    public:
        explicit AmplifyForm(QWidget *parent = nullptr);
        ~AmplifyForm();

        template<class T>
        void setAmpRange(T min, T max);

    private:
        Ui::AmplifyForm *ui;
    };
}
#endif // AMPLIFYFORM_H
