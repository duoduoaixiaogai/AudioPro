#include "WaveClip.h"
#include "Envelope.h"
#include "Sequence.h"

namespace RF {

    WaveClip::WaveClip(const std::shared_ptr<DirManager> &projDirManager,
                       sampleFormat format, int rate, int colourIndex)
    {
       mRate = rate;
       mColourIndex = colourIndex;
       mSequence = std::make_unique<Sequence>(projDirManager, format);

       mEnvelope = std::make_unique<Envelope>(true, 1e-7, 2.0, 1.0);

//       mWaveCache = std::make_unique<WaveCache>();
//       mSpecCache = std::make_unique<SpecCache>();
//       mSpecPxCache = std::make_unique<SpecPxCache>(1);
    }

    void WaveClip::SetOffset(double offset)
    {
        mOffset = offset;
        mEnvelope->SetOffset(mOffset);
    }

    void WaveClip::Append(samplePtr buffer, sampleFormat format,
                          size_t len, unsigned int stride,
                          XMLWriter* blockFileLog)
    {
       auto maxBlockSize = mSequence->GetMaxBlockSize();
       auto blockSize = mSequence->GetIdealAppendLen();
       sampleFormat seqFormat = mSequence->GetSampleFormat();

       if (!mAppendBuffer.ptr())
          mAppendBuffer.Allocate(maxBlockSize, seqFormat);

       auto cleanup = finally( [&] {
          // use NOFAIL-GUARANTEE
          UpdateEnvelopeTrackLen();
          MarkChanged();
       } );

       for(;;) {
          if (mAppendBufferLen >= blockSize) {
             // flush some previously appended contents
             // use STRONG-GUARANTEE
             mSequence->Append(mAppendBuffer.ptr(), seqFormat, blockSize,
                               blockFileLog);

             // use NOFAIL-GUARANTEE for rest of this "if"
             memmove(mAppendBuffer.ptr(),
                     mAppendBuffer.ptr() + blockSize * SAMPLE_SIZE(seqFormat),
                     (mAppendBufferLen - blockSize) * SAMPLE_SIZE(seqFormat));
             mAppendBufferLen -= blockSize;
             blockSize = mSequence->GetIdealAppendLen();
          }

          if (len == 0)
             break;

          // use NOFAIL-GUARANTEE for rest of this "for"
//          wxASSERT(mAppendBufferLen <= maxBlockSize);
          auto toCopy = std::min(len, maxBlockSize - mAppendBufferLen);

          CopySamples(buffer, format,
                      mAppendBuffer.ptr() + mAppendBufferLen * SAMPLE_SIZE(seqFormat),
                      seqFormat,
                      toCopy,
                      true, // high quality
                      stride);

          mAppendBufferLen += toCopy;
          buffer += toCopy * SAMPLE_SIZE(format) * stride;
          len -= toCopy;
       }
    }

    void WaveClip::UpdateEnvelopeTrackLen()
    // NOFAIL-GUARANTEE
    {
       mEnvelope->SetTrackLen
          ((mSequence->GetNumSamples().as_double()) / mRate, 1.0 / GetRate());
    }

    void WaveClip::Flush()
    // NOFAIL-GUARANTEE that the clip will be in a flushed state.
    // PARTIAL-GUARANTEE in case of exceptions:
    // Some initial portion (maybe none) of the append buffer of the
    // clip gets appended; no previously flushed contents are lost.
    {
       //wxLogDebug(wxT("WaveClip::Flush"));
       //wxLogDebug(wxT("   mAppendBufferLen=%lli"), (long long) mAppendBufferLen);
       //wxLogDebug(wxT("   previous sample count %lli"), (long long) mSequence->GetNumSamples());

       if (mAppendBufferLen > 0) {

          auto cleanup = finally( [&] {
             // Blow away the append buffer even in case of failure.  May lose some
             // data but don't leave the track in an un-flushed state.

             // Use NOFAIL-GUARANTEE of these steps.
             mAppendBufferLen = 0;
             UpdateEnvelopeTrackLen();
             MarkChanged();
          } );

          mSequence->Append(mAppendBuffer.ptr(), mSequence->GetSampleFormat(),
             mAppendBufferLen);
       }

       //wxLogDebug(wxT("now sample count %lli"), (long long) mSequence->GetNumSamples());
    }

    double WaveClip::GetEndTime() const
    {
       auto numSamples = mSequence->GetNumSamples();

       double maxLen = mOffset + (numSamples+mAppendBufferLen).as_double()/mRate;
       // JS: calculated value is not the length;
       // it is a maximum value and can be negative; no clipping to 0

       return maxLen;
    }
}
