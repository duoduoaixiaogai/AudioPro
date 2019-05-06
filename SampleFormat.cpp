#include "SampleFormat.h"
#include "Dither.h"

namespace RF {
    static DitherType gLowQualityDither = DitherType::none;
    static DitherType gHighQualityDither = DitherType::none;
    static Dither gDitherAlgorithm;

    void CopySamples(samplePtr src, sampleFormat srcFormat,
                     samplePtr dst, sampleFormat dstFormat,
                     unsigned int len,
                     bool highQuality,
                     unsigned int srcStride,
                     unsigned int dstStride)
    {
       gDitherAlgorithm.Apply(
          highQuality ? gHighQualityDither : gLowQualityDither,
          src, srcFormat, dst, dstFormat, len, srcStride, dstStride);
    }

    void ClearSamples(samplePtr dst, sampleFormat format,
                      size_t start, size_t len)
    {
       auto size = SAMPLE_SIZE(format);
       memset(dst + start*size, 0, len*size);
    }
}
