#ifndef IMPORTRAWDIALOG_H
#define IMPORTRAWDIALOG_H

#include "memoryx.h"

#include <QDialog>

namespace Ui {
    class ImportRawDialog;
}

namespace Renfeng {
    class ImportRawDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit ImportRawDialog(int encoding, unsigned channels,
                                 int offset, double rate,
                                 QWidget *parent = nullptr);
        ~ImportRawDialog();

        int mEncoding;
        unsigned mChannels;
        int mOffset;
        double mRate;
        double mPercent;
    private slots:
        void comboBoxChanged(int index);

        void on_buttonBox_accepted();

        void on_buttonBox_rejected();

    private:
        Ui::ImportRawDialog *ui;
        int mNumEncodings;
        ArrayOf<int> mEncodingSubtype;
    };
}

#endif // IMPORTRAWDIALOG_H
