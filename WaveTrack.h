#ifndef WAVETRACK_H
#define WAVETRACK_H

#include "SampleFormat.h"
#include "WaveClip.h"
#include "track.h"

#include <vector>

namespace RF {

    class WaveTrack final : public PlayableTrack {

    public:
        using Holder = std::unique_ptr<WaveTrack>;
        size_t GetMaxBlockSize() const;
        int GetWaveColorIndex() const { return mWaveColorIndex; };
        void Append(samplePtr buffer, sampleFormat format,
                       size_t len, unsigned int stride=1,
                       XMLWriter* blockFileLog=NULL);
        void Flush();
    private:
        friend class TrackFactory;
        WaveTrack(const std::shared_ptr<DirManager> &projDirManager,
                  sampleFormat format = (sampleFormat)0,
                  double rate = 0);
         WaveClip* RightmostOrNewClip();
         WaveClip* CreateClip();
    private:
        double mLegacyProjectFileOffset;
        int mAutoSaveIdent;
    protected:

        WaveClipHolders mClips;

        sampleFormat  mFormat;
        int           mRate;
        float         mGain;
        float         mPan;
        float         mOldGain[2];
        int           mWaveColorIndex;

        mutable float         mDisplayMin;
        mutable float         mDisplayMax;
        mutable float         mSpectrumMin;
        mutable float         mSpectrumMax;

        mutable int   mLastScaleType;
        mutable int           mLastdBRange;
    };
}

#endif
