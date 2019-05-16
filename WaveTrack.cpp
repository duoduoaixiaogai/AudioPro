#include "WaveTrack.h"
#include "project.h"
#include "Sequence.h"
#include "WaveClip.h"
#include "Envelope.h"

namespace Renfeng {
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

    Track::Holder WaveTrack::Duplicate() const
    {
       return std::make_shared<WaveTrack>( *this );
    }

    double WaveTrack::GetStartTime() const
    {
       bool found = false;
       double best = 0.0;

       if (mClips.empty())
          return 0;

       for (const auto &clip : mClips)
          if (!found)
          {
             found = true;
             best = clip->GetStartTime();
          }
          else if (clip->GetStartTime() < best)
             best = clip->GetStartTime();

       return best;
    }

    sampleCount WaveTrack::TimeToLongSamples(double t0) const
    {
       return sampleCount( floor(t0 * mRate + 0.5) );
    }

    auto WaveTrack::GetChannel() const -> ChannelType
    {
       if( mChannel != Track::MonoChannel )
          return mChannel;
       auto pan = GetPan();
       if( pan < -0.99 )
          return Track::LeftChannel;
       if( pan >  0.99 )
          return Track::RightChannel;
       return mChannel;
    }

    float WaveTrack::GetPan() const
    {
       return mPan;
    }

    bool WaveTrack::Get(samplePtr buffer, sampleFormat format,
                        sampleCount start, size_t len, fillFormat fill,
                        bool mayThrow, sampleCount * pNumCopied) const
    {
       // Simple optimization: When this buffer is completely contained within one clip,
       // don't clear anything (because we won't have to). Otherwise, just clear
       // everything to be on the safe side.
       bool doClear = true;
       bool result = true;
       sampleCount samplesCopied = 0;
       for (const auto &clip: mClips)
       {
          if (start >= clip->GetStartSample() && start+len <= clip->GetEndSample())
          {
             doClear = false;
             break;
          }
       }
       if (doClear)
       {
          // Usually we fill in empty space with zero
          if( fill == fillZero )
             ClearSamples(buffer, format, 0, len);
          // but we don't have to.
          else if( fill==fillTwo )
          {
//             wxASSERT( format==floatSample );
             float * pBuffer = (float*)buffer;
             for(size_t i=0;i<len;i++)
                pBuffer[i]=2.0f;
          }
          else
          {
//             wxFAIL_MSG(wxT("Invalid fill format"));
          }
       }

       for (const auto &clip: mClips)
       {
          auto clipStart = clip->GetStartSample();
          auto clipEnd = clip->GetEndSample();

          if (clipEnd > start && clipStart < start+len)
          {
             // Clip sample region and Get/Put sample region overlap
             auto samplesToCopy =
                std::min( start+len - clipStart, clip->GetNumSamples() );
             auto startDelta = clipStart - start;
             decltype(startDelta) inclipDelta = 0;
             if (startDelta < 0)
             {
                inclipDelta = -startDelta; // make positive value
                samplesToCopy -= inclipDelta;
                // samplesToCopy is now either len or
                //    (clipEnd - clipStart) - (start - clipStart)
                //    == clipEnd - start > 0
                // samplesToCopy is not more than len
                //
                startDelta = 0;
                // startDelta is zero
             }
             else {
                // startDelta is nonnegative and less than than len
                // samplesToCopy is positive and not more than len
             }

             if (!clip->GetSamples(
                   (samplePtr)(((char*)buffer) +
                               startDelta.as_size_t() *
                               SAMPLE_SIZE(format)),
                   format, inclipDelta, samplesToCopy.as_size_t(), mayThrow ))
                result = false;
             else
                samplesCopied += samplesToCopy;
          }
       }
       if( pNumCopied )
          *pNumCopied = samplesCopied;
       return result;
    }

    void WaveTrack::Set(samplePtr buffer, sampleFormat format,
                        sampleCount start, size_t len)
    // WEAK-GUARANTEE
    {
       for (const auto &clip: mClips)
       {
          auto clipStart = clip->GetStartSample();
          auto clipEnd = clip->GetEndSample();

          if (clipEnd > start && clipStart < start+len)
          {
             // Clip sample region and Get/Put sample region overlap
             auto samplesToCopy =
                std::min( start+len - clipStart, clip->GetNumSamples() );
             auto startDelta = clipStart - start;
             decltype(startDelta) inclipDelta = 0;
             if (startDelta < 0)
             {
                inclipDelta = -startDelta; // make positive value
                samplesToCopy -= inclipDelta;
                // samplesToCopy is now either len or
                //    (clipEnd - clipStart) - (start - clipStart)
                //    == clipEnd - start > 0
                // samplesToCopy is not more than len
                //
                startDelta = 0;
                // startDelta is zero
             }
             else {
                // startDelta is nonnegative and less than than len
                // samplesToCopy is positive and not more than len
             }

             clip->SetSamples(
                   (samplePtr)(((char*)buffer) +
                               startDelta.as_size_t() *
                               SAMPLE_SIZE(format)),
                              format, inclipDelta, samplesToCopy.as_size_t() );
             clip->MarkChanged();
          }
       }
    }
}
