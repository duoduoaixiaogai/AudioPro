#include "amplify.h"
#include "track.h"
#include "WaveTrack.h"
#include "amplifyform.h"

namespace Renfeng {

#define DB_TO_LINEAR(x) (pow(10.0, (x) / 20.0))
#define LINEAR_TO_DB(x) (20.0 * log10(x))

    Param( Ratio,     float,   ("Ratio"),            0.9f,       0.003162f,  316.227766f,   1.0f  );
    Param( Amp,       float,   (""),                -0.91515f,  -50.0f,     50.0f,         10.0f );
    Param( Clipping,  bool,    ("AllowClipping"),    false,    false,  true,    1  );

    EffectAmplify::EffectAmplify() {
        mAmp = DEF_Amp;
        mRatio = DB_TO_LINEAR(mAmp);
        mRatioClip = 0.0;
        mCanClip = false;
        mPeak =0.0;
    }

    EffectAmplify::~EffectAmplify() {

    }

    bool EffectAmplify::Init()
    {
        mPeak = 0.0;

        //for (auto t : inputTracks()->Selected< const WaveTrack >())
        for (auto t : inputTracks()->Selected< const WaveTrack >())
            {
                // test
                mT1 = 42.956916099773245;

                auto pair = t->GetMinMax(mT0, mT1); // may throw
                const float min = pair.first, max = pair.second;
                float newpeak = (fabs(min) > fabs(max) ? fabs(min) : fabs(max));

                if (newpeak > mPeak)
                    {
                        mPeak = newpeak;
                    }
            }

        return true;
    }

    ComponentInterfaceSymbol EffectAmplify::getSymbol()
    {
        return AMPLIFY_PLUGIN_SYMBOL;
    }

    unsigned EffectAmplify::GetAudioInCount()
    {
        return 1;
    }

    unsigned EffectAmplify::GetAudioOutCount()
    {
        return 1;
    }

    void EffectAmplify::PopulateOrExchange(QWidget *parent)
    {
        if (IsBatchProcessing())
            {
                mPeak = 1.0;
            }
        else
            {
                if (mPeak > 0.0)
                    {
                        mRatio = 1.0 / mPeak;
                        mRatioClip = mRatio;
                    }
                else
                    {
                        mRatio = 1.0;
                    }
            }
        //    mForm = new AmplifyForm(parent);

        //        AmplifyForm* form = dynamic_cast<AmplifyForm*>(parent);
        mForm = new AmplifyForm(parent);

        if (!mForm) {
                return;
            }

        connect(mForm->ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(OnAmpSlider(int)));
        connect(mForm->ui->lineEdit, SIGNAL(textChanged(const QString &text)), this, SLOT(OnAmpText(const QString &text)));
        connect(mForm->ui->lineEdit_2, SIGNAL(textChanged(const QString &text)), this, SLOT(OnPeakText(const QString &text)));
        connect(mForm->ui->pushButton, SIGNAL(clicked()), this, SLOT(apply()));

        mForm->setAmpRange(MIN_Amp, MAX_Amp);
        mForm->setSliderRange(MIN_Amp * SCL_Amp, MAX_Amp * SCL_Amp);

        double minAmp = MIN_Amp + LINEAR_TO_DB(mPeak);
        double maxAmp = MAX_Amp + LINEAR_TO_DB(mPeak);
        minAmp = QString("%1").arg(minAmp, 9, 'f', 4).toDouble();
        maxAmp = QString("%1").arg(maxAmp, 9, 'f', 4).toDouble();
        mForm->setPeakRange(minAmp, maxAmp);

        mForm->setClipSel(false);
        if (IsBatchProcessing())
            {
                mForm->setClipEnable(false);
                mCanClip = true;
            }

        double dBInit = LINEAR_TO_DB(mRatio);
        double dB = TrapDouble(dBInit, MIN_Amp, MAX_Amp);
        if (dB != dBInit)
            mRatio = DB_TO_LINEAR(dB);

        mAmp = LINEAR_TO_DB(mRatio);
        mForm->setAmpValue(mAmp);

        mForm->setSliderValue((int) (mAmp * SCL_Amp + 0.5f));

        mNewPeak = LINEAR_TO_DB(mRatio * mPeak);
        mForm->setPeakValue(mNewPeak);
    }

    void EffectAmplify::CheckClip(QWidget *amplifyForm)
    {
        //        AmplifyForm* form = dynamic_cast<AmplifyForm*>(amplifyForm);
        //
        //        if (!form) {
        //                return;
        //            }

        //       EnableApply(form->getClipSel() || (mPeak > 0.0 && mRatio <= mRatioClip));
    }

    void EffectAmplify::OnAmpSlider(int value)
    {
       double dB = value / SCL_Amp;
       mRatio = DB_TO_LINEAR(TrapDouble(dB, MIN_Amp, MAX_Amp));

       double dB2 = (value - 1) / SCL_Amp;
       double ratio2 = DB_TO_LINEAR(TrapDouble(dB2, MIN_Amp, MAX_Amp));

       if (!mForm->getClipSel() && mRatio * mPeak > 1.0 && ratio2 * mPeak < 1.0)
       {
          mRatio = 1.0 / mPeak;
       }

       mAmp = LINEAR_TO_DB(mRatio);
       mForm->setAmpValue(mAmp);

       mNewPeak = LINEAR_TO_DB(mRatio * mPeak);
       mForm->setPeakValue(mNewPeak);

//       CheckClip();
    }

    void EffectAmplify::OnAmpText(const QString &value)
    {
//       if (!mAmpT->GetValidator()->TransferFromWindow())
//       {
//          EnableApply(false);
//          return;
//       }

       mRatio = DB_TO_LINEAR(TrapDouble(mAmp * SCL_Amp, MIN_Amp * SCL_Amp, MAX_Amp * SCL_Amp) / SCL_Amp);

       mForm->setSliderValue((int) (LINEAR_TO_DB(mRatio) * SCL_Amp + 0.5));

       mNewPeak = LINEAR_TO_DB(mRatio * mPeak);
       mForm->setPeakValue(mNewPeak);

//       CheckClip();
    }

    void EffectAmplify::OnPeakText(const QString &value)
    {
//       if (!mNewPeakT->GetValidator()->TransferFromWindow())
//       {
//          EnableApply(false);
//          return;
//       }

       if (mNewPeak == 0.0)
          mRatio = mRatioClip;
       else
          mRatio = DB_TO_LINEAR(mNewPeak) / mPeak;

       double ampInit = LINEAR_TO_DB(mRatio);
       mAmp = TrapDouble(ampInit, MIN_Amp, MAX_Amp);
       if (mAmp != ampInit)
          mRatio = DB_TO_LINEAR(mAmp);

       mForm->setAmpValue(mAmp);

       mForm->setSliderValue((int) (mAmp * SCL_Amp + 0.5f));

//       CheckClip();
    }

    void EffectAmplify::apply() {
      mRatio = DB_TO_LINEAR(TrapDouble(mAmp * SCL_Amp, MIN_Amp * SCL_Amp, MAX_Amp * SCL_Amp) / SCL_Amp);

         mCanClip = mForm->getClipSel();

         if (!mCanClip && mRatio * mPeak > 1.0)
         {
            mRatio = 1.0 / mPeak;
         }

         dynamic_cast<QDialog*>(mForm->parent())->reject();
    }
}
