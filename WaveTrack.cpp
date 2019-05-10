#include "WaveTrack.h"
#include "project.h"
#include "Sequence.h"
#include "WaveClip.h"
#include "Envelope.h"

namespace RF {
    WaveTrack::Holder TrackFactory::NewWaveTrack(sampleFormat format, double rate)
    {
        return std::unique_ptr<WaveTrack>
        { new WaveTrack(mDirManager, format, rate) };
    }

    WaveTrack::WaveTrack(const std::shared_ptr<DirManager> &projDirManager, sampleFormat format, double rate) :
        PlayableTrack(projDirManager)
    {
        if (format == (sampleFormat)0)
        {
            format = GetActiveProject()->GetDefaultFormat();
        }
        if (rate == 0)
        {
            rate = GetActiveProject()->GetRate();
        }

        // Force creation always:
        //       WaveformSettings &settings = GetIndependentWaveformSettings();
        //
        //       mDisplay = TracksPrefs::ViewModeChoice();
        //       if (mDisplay == obsoleteWaveformDBDisplay) {
        //          mDisplay = Waveform;
        //          settings.scaleType = WaveformSettings::stLogarithmic;
        //       }

        mLegacyProjectFileOffset = 0;

        mFormat = format;
        mRate = (int) rate;
        mGain = 1.0;
        mPan = 0.0;
        mOldGain[0] = 0.0;
        mOldGain[1] = 0.0;
        mWaveColorIndex = 0;
        //       SetDefaultName(TracksPrefs::GetDefaultAudioTrackNamePreference());
        //       SetName(GetDefaultName());
        mDisplayMin = -1.0;
        mDisplayMax = 1.0;
        mSpectrumMin = mSpectrumMax = -1; // so values will default to settings
        mLastScaleType = -1;
        mLastdBRange = -1;
        mAutoSaveIdent = 0;

        //       SetHeight( TrackInfo::DefaultWaveTrackHeight() );
    }

    size_t WaveTrack::GetMaxBlockSize() const
    {
        decltype(GetMaxBlockSize()) maxblocksize = 0;
        for (const auto &clip : mClips)
        {
            maxblocksize = std::max(maxblocksize, clip->GetSequence()->GetMaxBlockSize());
        }

        if (maxblocksize == 0)
        {
            // We really need the maximum block size, so create a
            // temporary sequence to get it.
            maxblocksize = Sequence{ mDirManager, mFormat }.GetMaxBlockSize();
        }

        //       wxASSERT(maxblocksize > 0);

        return maxblocksize;
    }

    void WaveTrack::Append(samplePtr buffer, sampleFormat format,
                           size_t len, unsigned int stride /* = 1 */,
                           XMLWriter *blockFileLog /* = NULL */)
    // PARTIAL-GUARANTEE in case of exceptions:
    // Some prefix (maybe none) of the buffer is appended, and no content already
    // flushed to disk is lost.
    {
        RightmostOrNewClip()->Append(buffer, format, len, stride,
                                     blockFileLog);
    }

    WaveClip* WaveTrack::RightmostOrNewClip()
    // NOFAIL-GUARANTEE
    {
        if (mClips.empty()) {
            WaveClip *clip = CreateClip();
            clip->SetOffset(mOffset);
            return clip;
        }
        else
        {
            auto it = mClips.begin();
            WaveClip *rightmost = (*it++).get();
            double maxOffset = rightmost->GetOffset();
            for (auto end = mClips.end(); it != end; ++it)
            {
                WaveClip *clip = it->get();
                double offset = clip->GetOffset();
                if (maxOffset < offset)
                    maxOffset = offset, rightmost = clip;
            }
            return rightmost;
        }
    }

    WaveClip* WaveTrack::CreateClip()
    {
        mClips.push_back(std::make_unique<WaveClip>(mDirManager, mFormat, mRate, GetWaveColorIndex()));
        return mClips.back().get();
    }

    void WaveTrack::Flush()
    // NOFAIL-GUARANTEE that the rightmost clip will be in a flushed state.
    // PARTIAL-GUARANTEE in case of exceptions:
    // Some initial portion (maybe none) of the append buffer of the rightmost
    // clip gets appended; no previously saved contents are lost.
    {
        // After appending, presumably.  Do this to the clip that gets appended.
        RightmostOrNewClip()->Flush();
    }

    double WaveTrack::GetRate() const
    {
       return mRate;
    }

    double WaveTrack::GetEndTime() const
    {
       bool found = false;
       double best = 0.0;

       if (mClips.empty())
          return 0;

       for (const auto &clip : mClips)
          if (!found)
          {
             found = true;
             best = clip->GetEndTime();
          }
          else if (clip->GetEndTime() > best)
             best = clip->GetEndTime();

       return best;
    }

    std::pair<float, float> WaveTrack::GetMinMax(
       double t0, double t1, bool mayThrow) const
    {
       std::pair<float, float> results {
          // we need these at extremes to make sure we find true min and max
          FLT_MAX, -FLT_MAX
       };
       bool clipFound = false;

       if (t0 > t1) {
//          if (mayThrow)
//             THROW_INCONSISTENCY_EXCEPTION;
          return results;
       }

       if (t0 == t1)
          return results;

       for (const auto &clip: mClips)
       {
          if (t1 >= clip->GetStartTime() && t0 <= clip->GetEndTime())
          {
             clipFound = true;
             auto clipResults = clip->GetMinMax(t0, t1, mayThrow);
             if (clipResults.first < results.first)
                results.first = clipResults.first;
             if (clipResults.second > results.second)
                results.second = clipResults.second;
          }
       }

       if(!clipFound)
       {
          results = { 0.f, 0.f }; // sensible defaults if no clips found
       }

       return results;
    }
}
