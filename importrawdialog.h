#ifndef IMPORTRAWDIALOG_H
#define IMPORTRAWDIALOG_H

#include "memoryx.h"

#include <QDialog>

namespace Ui {
    class ImportRawDialog;
}

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

private:
    Ui::ImportRawDialog *ui;
    int mNumEncoding;
    ArrayOf<int> mEncodingSubtype;
};

#endif // IMPORTRAWDIALOG_H
