#include "amplify.h"
#include "track.h"
#include "WaveTrack.h"

namespace RF {
    EffectAmplify::EffectAmplify() {

    }

    EffectAmplify::~EffectAmplify() {

    }

    bool EffectAmplify::Init()
    {
       mPeak = 0.0;

       for (auto t : inputTracks()->Selected< const WaveTrack >())
       {
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
}
