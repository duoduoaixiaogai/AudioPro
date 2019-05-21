#ifndef MIX_H
#define MIX_H

#include "SampleFormat.h"

#include <vector>
#include <memory>

namespace Renfeng {

  class WaveTrack;
  class TimeTrack;
  class WaveTrackCache;
  class Resample;

  using WaveTrackConstArray = std::vector < std::shared_ptr < const WaveTrack > >;

  class MixerSpec
  {
    unsigned mNumTracks, mNumChannels, mMaxNumChannels;
  public:
    unsigned GetNumChannels() { return mNumChannels; }
    unsigned GetNumTracks() { return mNumTracks; }
    ArraysOf<bool> mMap;
  };

  class Mixer {
  public:
      class WarpOptions
      {
      public:
          explicit WarpOptions(const TimeTrack *t)
              : timeTrack(t), minSpeed(0.0), maxSpeed(0.0)
          {}

          WarpOptions(double min, double max);

      private:
          friend class Mixer;
          const TimeTrack *timeTrack;
          double minSpeed, maxSpeed;
      };

      Mixer(const WaveTrackConstArray &inputTracks, bool mayThrow,
            const WarpOptions &warpOptions,
            double startTime, double stopTime,
            unsigned numOutChannels, size_t outBufferSize, bool outInterleaved,
            double outRate, sampleFormat outFormat,
            bool highQuality = true, MixerSpec *mixerSpec = NULL);

      virtual ~ Mixer();
      size_t Process(size_t maxSamples);
      samplePtr GetBuffer();
      double MixGetCurrentTime();
  private:
      void MakeResamplers();
      void Clear();
      size_t MixSameRate(int *channelFlags, WaveTrackCache &cache,
                                 sampleCount *pos);
      size_t MixVariableRates(int *channelFlags, WaveTrackCache &cache,
                                      sampleCount *pos, float *queue,
                                      int *queueStart, int *queueLen,
                                      Resample * pResample);
  private:
      size_t           mNumInputTracks;
      size_t           mQueueMaxLen;
      FloatBuffers     mSampleQueue;
      bool             mApplyTrackGains;
      MixerSpec        *mMixerSpec;
      size_t           mProcessLen;
      ArrayOf<int>     mQueueStart;
      ArrayOf<int>     mQueueLen;
//      ArrayOf<std::unique_ptr<Resample>> mResample;
      bool             mbVariableRates;
      Doubles          mEnvValues;
      // Output
      unsigned         mNumChannels;
      Floats           mGains;
      size_t              mBufferSize;
      bool             mInterleaved;
      double           mRate;
      double           mSpeed;
      sampleFormat     mFormat;
      unsigned         mNumBuffers;
      size_t              mInterleavedBufferSize;
      ArrayOf<SampleBuffer> mBuffer, mTemp;
      Floats           mFloatBuffer;
      std::vector<double> mMinFactor, mMaxFactor;
      size_t              mMaxOut;

      bool             mMayThrow;
      bool             mHighQuality;
      ArrayOf<WaveTrackCache> mInputTrack;
      ArrayOf<sampleCount> mSamplePos;
      const TimeTrack *mTimeTrack;
      double           mT0; // Start time
      double           mT1; // Stop time (none if mT0==mT1)
      double           mTime;  // Current time (renamed from mT to mTime for consistency with AudioIO - mT represented warped time there)
  };
}

#endif // MIX_H
