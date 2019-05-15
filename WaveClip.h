#ifndef WAVECLIP_H
#define WAVECLIP_H

#include "XMLTagHandler.h"
#include "SampleFormat.h"

#include <vector>

namespace Renfeng {

    class Sequence;
    class DirManager;
    class Envelope;

    class WaveClip final : public XMLTagHandler {
    public:
        Sequence* GetSequence() { return mSequence.get(); }
        WaveClip(const std::shared_ptr<DirManager> &projDirManager, sampleFormat format,
                 int rate, int colourIndex);
        void SetOffset(double offset);
        double GetOffset() const { return mOffset; }
        void Append(samplePtr buffer, sampleFormat format,
                    size_t len, unsigned int stride=1,
                    XMLWriter* blockFileLog = NULL);
        void UpdateEnvelopeTrackLen();
        void MarkChanged() // NOFAIL-GUARANTEE
        { mDirty++; }
        void Flush();
        int GetRate() const { return mRate; }
        double GetEndTime() const;
        double GetStartTime() const;
        std::pair<float, float> GetMinMax(
              double t0, double t1, bool mayThrow = true) const;
        void TimeToSamplesClip(double t0, sampleCount *s0) const;
        sampleCount GetStartSample() const;
        sampleCount GetEndSample() const;
        sampleCount GetNumSamples() const;
        bool GetSamples(samplePtr buffer, sampleFormat format,
                           sampleCount start, size_t len, bool mayThrow = true) const;
    protected:
        std::unique_ptr<Sequence> mSequence;
        double mOffset { 0 };
        std::unique_ptr<Envelope> mEnvelope;

        SampleBuffer  mAppendBuffer {};
        int mDirty { 0 };
        size_t        mAppendBufferLen { 0 };
        int mRate;
        int mColourIndex;
    };

    using WaveClipHolder = std::shared_ptr< WaveClip >;
    using WaveClipHolders = std::vector < WaveClipHolder >;
}

#endif
