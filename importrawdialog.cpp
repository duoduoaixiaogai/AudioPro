#include "importrawdialog.h"
#include "ui_importrawdialog.h"
#include "FileFormats.h"

#include <QStringList>
#include <QPushButton>

namespace RF {
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

        num = sf_num_encodings();
        mNumEncodings = 0;
        mEncodingSubtype.reinit(static_cast<size_t>(num));
        selection = 0;
        for (i=0; i<num; i++) {
            SF_INFO info;
            memset(&info, 0, sizeof(SF_INFO));
            int subtype = sf_encoding_index_to_subtype(i);
            info.format = SF_FORMAT_RAW + SF_ENDIAN_LITTLE + subtype;
            info.channels = 1;
            info.samplerate = 44100;

            if (sf_format_check(&info)) {
                mEncodingSubtype[mNumEncodings] = subtype;
                encodings.push_back(sf_encoding_index_name(i));
                if ((mEncoding & SF_FORMAT_SUBMASK) == subtype)
                    selection = mNumEncodings;

                mNumEncodings++;
            }
        }

        endians.push_back(QString("No endianness"));
        endians.push_back(QString("Little-endian"));
        endians.push_back(QString("Big-endian"));
        endians.push_back(QString("Default endianness"));

        switch (mEncoding & (SF_FORMAT_ENDMASK))
        {
            default:
            case SF_ENDIAN_FILE:
                endian = 0;
                break;
            case SF_ENDIAN_LITTLE:
                endian = 1;
                break;
            case SF_ENDIAN_BIG:
                endian = 2;
                break;
            case SF_ENDIAN_CPU:
                endian = 3;
                break;
        }

        chans.push_back(QString("1 Channel (Mono)"));
        chans.push_back(QString("2 Channels (Stereo)"));
        for (i=2; i<16; i++) {
            chans.push_back(QString("%1 Channels").arg( i + 1));
        }

        ui->comboBox->addItems(encodings);
        ui->comboBox->setCurrentIndex(selection);

        ui->comboBox_2->addItems(endians);
        ui->comboBox_2->setCurrentIndex(endian);

        ui->comboBox_3->addItems(chans);
        ui->comboBox_3->setCurrentIndex(mChannels - 1);

        ui->lineEdit->setText(QString("%1").arg(mOffset));
        ui->lineEdit_2->setText(QString("%1").arg(100));
        ui->lineEdit_3->setText(QString("%1").arg(mRate));

        connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged(int)));
        connect(ui->comboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged(int)));
        connect(ui->comboBox_3, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged(int)));
    }

    ImportRawDialog::~ImportRawDialog()
    {
        delete ui;
    }

    void ImportRawDialog::comboBoxChanged(int index) {
        SF_INFO info;
        memset(&info, 0, sizeof(SF_INFO));
        mEncoding = mEncodingSubtype[ui->comboBox->currentIndex()];
        mEncoding += (ui->comboBox_2->currentIndex() * 0x10000000);

        info.format = mEncoding | SF_FORMAT_RAW;
        info.channels = ui->comboBox_3->currentIndex() + 1;
        info.samplerate = 44100;

        if (sf_format_check(&info)) {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            return;
        }
        info.channels = 1;
        if (sf_format_check(&info)) {
            ui->comboBox_3->setCurrentIndex(0);
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            return;
        }
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }

    void ImportRawDialog::on_buttonBox_accepted() {

        mEncoding = mEncodingSubtype[ui->comboBox->currentIndex()];
        mEncoding += (ui->comboBox_2->currentIndex() * 0x10000000);
        mChannels = ui->comboBox_3->currentIndex() + 1;
        long l = ui->lineEdit->text().toLong();
        mOffset = l;
        mPercent = ui->lineEdit_2->text().toDouble();
        mRate = ui->lineEdit_3->text().toDouble();

        if (mChannels < 1 || mChannels > 16)
            mChannels = 1;
        if (mOffset < 0)
            mOffset = 0;
        if (mPercent < 0.0)
            mPercent = 0.0;
        if (mPercent > 100.0)
            mPercent = 100.0;
        if (mRate < 100.0)
            mRate = 100.0;
        // Highest preset sample rate supported in Audacity 2.3.0 is 384 kHz
        if (mRate > 384000.0)
            mRate = 384000.0;

        setResult(QDialog::Accepted);
    }

    void ImportRawDialog::on_buttonBox_rejected() {
        setResult(QDialog::Rejected);
    }
}
