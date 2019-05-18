#include "resample.h"

namespace Renfeng {
//  Resample::Resample(const bool useBestMethod, const double dMinFactor, const double dMaxFactor)
//  {
//     this->SetMethod(useBestMethod);
//     soxr_quality_spec_t q_spec;
//     if (dMinFactor == dMaxFactor)
//     {
//        mbWantConstRateResampling = true; // constant rate resampling
//        q_spec = soxr_quality_spec("\0\1\4\6"[mMethod], 0);
//     }
//     else
//     {
//        mbWantConstRateResampling = false; // variable rate resampling
//        q_spec = soxr_quality_spec(SOXR_HQ, SOXR_VR);
//     }
//     mHandle.reset(soxr_create(1, dMinFactor, 1, 0, 0, &q_spec, 0));
//  }
//
//  Resample::~Resample()
//  {
//  }
//
//  void Resample::SetMethod(const bool useBestMethod)
//  {
//     if (useBestMethod)
//        mMethod = BestMethodSetting.ReadInt();
//     else
//        mMethod = FastMethodSetting.ReadInt();
//  }
}
