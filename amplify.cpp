#include "amplify.h"
#include "track.h"
#include "WaveTrack.h"
#include "amplifyform.h"

namespace RF {

    Param( Ratio,     float,   ("Ratio"),            0.9f,       0.003162f,  316.227766f,   1.0f  );
    Param( Amp,       float,   (""),                -0.91515f,  -50.0f,     50.0f,         10.0f );
    Param( Clipping,  bool,    ("AllowClipping"),    false,    false,  true,    1  );

    EffectAmplify::EffectAmplify() {

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

    void EffectAmplify::PopulateOrExchange(QWidget *amplifyForm)
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

        AmplifyForm* form = dynamic_cast<AmplifyForm*>(amplifyForm);

        if (!form) {
            return;
        }


    }


}
