#include "importrawdialog.h"
#include "ui_importrawdialog.h"

#include <QStringList>

ImportRawDialog::ImportRawDialog(int encoding, unsigned channels,
                                 int offset, double rate,
                                 QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportRawDialog),
    mEncoding(encoding),
    mChannels(channels),
    mOffset(offset),
    mRate(rate)
{
    ui->setupUi(this);

    QStringList encodings, endians, chans;
    int num, selection, endian, i;

    num =
}

ImportRawDialog::~ImportRawDialog()
{
    delete ui;
}
