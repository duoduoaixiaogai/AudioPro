#include "WaveClip.h"
#include "Envelope.h"
#include "Sequence.h"

namespace Renfeng {

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

    double WaveClip::GetStartTime() const
    {
       // JS: mOffset is the minimum value and it is returned; no clipping to 0
       return mOffset;
    }

    std::pair<float, float> WaveClip::GetMinMax(
       double t0, double t1, bool mayThrow) const
    {
       if (t0 > t1) {
//          if (mayThrow)
//             THROW_INCONSISTENCY_EXCEPTION;
          return {
             0.f,  // harmless, but unused since Sequence::GetMinMax does not use these values
             0.f   // harmless, but unused since Sequence::GetMinMax does not use these values
          };
       }

       if (t0 == t1)
          return{ 0.f, 0.f };

       sampleCount s0, s1;

       TimeToSamplesClip(t0, &s0);
       TimeToSamplesClip(t1, &s1);

       return mSequence->GetMinMax(s0, s1-s0, mayThrow);
    }

    void WaveClip::TimeToSamplesClip(double t0, sampleCount *s0) const
    {
       if (t0 < mOffset)
          *s0 = 0;
       else if (t0 > mOffset + mSequence->GetNumSamples().as_double()/mRate)
          *s0 = mSequence->GetNumSamples();
       else
          *s0 = sampleCount( floor(((t0 - mOffset) * mRate) + 0.5) );
    }

    sampleCount WaveClip::GetStartSample() const
    {
       return sampleCount( floor(mOffset * mRate + 0.5) );
    }

    sampleCount WaveClip::GetEndSample() const
    {
       return GetStartSample() + mSequence->GetNumSamples();
    }

    sampleCount WaveClip::GetNumSamples() const
    {
       return mSequence->GetNumSamples();
    }

    bool WaveClip::GetSamples(samplePtr buffer, sampleFormat format,
                       sampleCount start, size_t len, bool mayThrow) const
    {
       return mSequence->Get(buffer, format, start, len, mayThrow);
    }
}
