#ifndef WAVETRACK_H
#define WAVETRACK_H

#include "track.h"
#include "SampleFormat.h"
#include "WaveClip.h"

#include <vector>

namespace Renfeng {

    class WaveTrack final : public PlayableTrack {

    public:
        using Holder = std::unique_ptr<WaveTrack>;
        size_t GetMaxBlockSize() const;
        int GetWaveColorIndex() const { return mWaveColorIndex; };
        void Append(samplePtr buffer, sampleFormat format,
                       size_t len, unsigned int stride=1,
                       XMLWriter* blockFileLog=NULL);
        void Flush();
        double GetRate() const;
        double GetEndTime() const override;
        std::pair<float, float> GetMinMax(
              double t0, double t1, bool mayThrow = true) const;
        double GetStartTime() const override;
        sampleCount TimeToLongSamples(double t0) const;
        ChannelType GetChannel() const override;
        float GetPan() const;
        bool Get(samplePtr buffer, sampleFormat format,
                           sampleCount start, size_t len,
                           fillFormat fill = fillZero, bool mayThrow = true, sampleCount * pNumCopied = nullptr) const;
        void Set(samplePtr buffer, sampleFormat format,
                           sampleCount start, size_t len);
        double GetOffset() const override;
        sampleCount GetBlockStart(sampleCount t) const;
        size_t GetBestBlockSize(sampleCount t) const;
        void GetEnvelopeValues(double *buffer, size_t bufferLen,
                                 double t0) const;
        float GetChannelGain(int channel) const;
    private:
        friend class TrackFactory;
        WaveTrack(const std::shared_ptr<DirManager> &projDirManager,
                  sampleFormat format = (sampleFormat)0,
                  double rate = 0);
         WaveClip* RightmostOrNewClip();
         WaveClip* CreateClip();
         TrackKind GetKind() const override { return TrackKind::Wave; }
         Track::Holder Duplicate() const override;
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

    class WaveTrackCache {
    public:
       WaveTrackCache()
          : mBufferSize(0)
          , mOverlapBuffer()
          , mNValidBuffers(0)
       {
       }

       explicit WaveTrackCache(const std::shared_ptr<const WaveTrack> &pTrack)
          : mBufferSize(0)
          , mOverlapBuffer()
          , mNValidBuffers(0)
       {
          SetTrack(pTrack);
       }
       ~WaveTrackCache();

       const std::shared_ptr<const WaveTrack>& GetTrack() const { return mPTrack; }
       void SetTrack(const std::shared_ptr<const WaveTrack> &pTrack);

       // Uses fillZero always
       // Returns null on failure
       // Returned pointer may be invalidated if Get is called again
       // Do not DELETE[] the pointer
       constSamplePtr Get(
          sampleFormat format, sampleCount start, size_t len, bool mayThrow);

    private:
       void Free();

       struct Buffer {
          Floats data;
          sampleCount start;
          sampleCount len;

          Buffer() : start(0), len(0) {}
          void Free() { data.reset(); start = 0; len = 0; }
          sampleCount end() const { return start + len; }

          void swap ( Buffer &other )
          {
             data .swap ( other.data );
             std::swap( start, other.start );
             std::swap( len, other.len );
          }
       };

       std::shared_ptr<const WaveTrack> mPTrack;
       size_t mBufferSize;
       Buffer mBuffers[2];
       GrowableSampleBuffer mOverlapBuffer;
       int mNValidBuffers;
    };
}

#endif
