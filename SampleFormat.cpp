#include "SampleFormat.h"
#include "Dither.h"

namespace Renfeng {
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

    void ReverseSamples(samplePtr dst, sampleFormat format,
                      int start, int len)
    {
       auto size = SAMPLE_SIZE(format);
       samplePtr first = dst + start * size;
       samplePtr last = dst + (start + len - 1) * size;
       enum : size_t { fixedSize = SAMPLE_SIZE(floatSample) };
//       wxASSERT(static_cast<size_t>(size) <= fixedSize);
       char temp[fixedSize];
       while (first < last) {
          memcpy(temp, first, size);
          memcpy(first, last, size);
          memcpy(last, temp, size);
          first += size;
          last -= size;
       }
    }
}
