/**********************************************************************

  Audacity: A Digital Audio Editor

  SpecPowerMeter.h

  Philipp Sibler

**********************************************************************/

#ifndef SPECPOWERMETER_H
#define SPECPOWERMETER_H

#include <cstddef>
#include "../SampleFormat.h"

namespace Renfeng {
    class SpecPowerCalculation
    {
        const size_t mSigLen;

        Floats mSigI;
        Floats mSigFR;
        Floats mSigFI;

        float CalcBinPower(float* sig_f_r, float* sig_f_i, int loBin, int hiBin);
        int Freq2Bin(float fc);
    public:
        SpecPowerCalculation(size_t sigLen);
        ~SpecPowerCalculation();

        float CalcPower(float* sig, float fc, float bw);
    };
}
#endif

