#ifndef WAVECLIP_H
#define WAVECLIP_H

#include "XMLTagHandler.h"
#include "SampleFormat.h"

#include <vector>

namespace RF {

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
    protected:
        std::unique_ptr<Sequence> mSequence;
        double mOffset { 0 };
        std::unique_ptr<Envelope> mEnvelope;

        SampleBuffer  mAppendBuffer {};
        int mDirty { 0 };
        size_t        mAppendBufferLen { 0 };
        int mRate;
    };

    using WaveClipHolder = std::shared_ptr< WaveClip >;
    using WaveClipHolders = std::vector < WaveClipHolder >;
}

#endif
