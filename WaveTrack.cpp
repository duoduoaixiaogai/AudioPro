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

    double WaveTrack::GetOffset() const
    {
       return GetStartTime();
    }

    sampleCount WaveTrack::GetBlockStart(sampleCount s) const
    {
       for (const auto &clip : mClips)
       {
          const auto startSample = (sampleCount)floor(0.5 + clip->GetStartTime()*mRate);
          const auto endSample = startSample + clip->GetNumSamples();
          if (s >= startSample && s < endSample)
             return startSample + clip->GetSequence()->GetBlockStart(s - startSample);
       }

       return -1;
    }

    size_t WaveTrack::GetBestBlockSize(sampleCount s) const
    {
       auto bestBlockSize = GetMaxBlockSize();

       for (const auto &clip : mClips)
       {
          auto startSample = (sampleCount)floor(clip->GetStartTime()*mRate + 0.5);
          auto endSample = startSample + clip->GetNumSamples();
          if (s >= startSample && s < endSample)
          {
             bestBlockSize = clip->GetSequence()->GetBestBlockSize(s - startSample);
             break;
          }
       }

       return bestBlockSize;
    }

    WaveTrackCache::~WaveTrackCache()
    {
    }

    void WaveTrackCache::SetTrack(const std::shared_ptr<const WaveTrack> &pTrack)
    {
       if (mPTrack != pTrack) {
          if (pTrack) {
             mBufferSize = pTrack->GetMaxBlockSize();
             if (!mPTrack ||
                 mPTrack->GetMaxBlockSize() != mBufferSize) {
                Free();
                mBuffers[0].data = Floats{ mBufferSize };
                mBuffers[1].data = Floats{ mBufferSize };
             }
          }
          else
             Free();
          mPTrack = pTrack;
          mNValidBuffers = 0;
       }
    }

    constSamplePtr WaveTrackCache::Get(sampleFormat format,
       sampleCount start, size_t len, bool mayThrow)
    {
       if (format == floatSample && len > 0) {
          const auto end = start + len;

          bool fillFirst = (mNValidBuffers < 1);
          bool fillSecond = (mNValidBuffers < 2);

          // Discard cached results that we no longer need
          if (mNValidBuffers > 0 &&
              (end <= mBuffers[0].start ||
               start >= mBuffers[mNValidBuffers - 1].end())) {
             // Complete miss
             fillFirst = true;
             fillSecond = true;
          }
          else if (mNValidBuffers == 2 &&
                   start >= mBuffers[1].start &&
                   end > mBuffers[1].end()) {
             // Request starts in the second buffer and extends past it.
             // Discard the first buffer.
             // (But don't deallocate the buffer space.)
             mBuffers[0] .swap ( mBuffers[1] );
             fillSecond = true;
             mNValidBuffers = 1;
          }
          else if (mNValidBuffers > 0 &&
             start < mBuffers[0].start &&
             0 <= mPTrack->GetBlockStart(start)) {
             // Request is not a total miss but starts before the cache,
             // and there is a clip to fetch from.
             // Not the access pattern for drawing spectrogram or playback,
             // but maybe scrubbing causes this.
             // Move the first buffer into second place, and later
             // refill the first.
             // (This case might be useful when marching backwards through
             // the track, as with scrubbing.)
             mBuffers[0] .swap ( mBuffers[1] );
             fillFirst = true;
             fillSecond = false;
             // Cache is not in a consistent state yet
             mNValidBuffers = 0;
          }

          // Refill buffers as needed
          if (fillFirst) {
             const auto start0 = mPTrack->GetBlockStart(start);
             if (start0 >= 0) {
                const auto len0 = mPTrack->GetBestBlockSize(start0);
//                wxASSERT(len0 <= mBufferSize);
                if (!mPTrack->Get(
                      samplePtr(mBuffers[0].data.get()), floatSample, start0, len0,
                      fillZero, mayThrow))
                   return 0;
                mBuffers[0].start = start0;
                mBuffers[0].len = len0;
                if (!fillSecond &&
                    mBuffers[0].end() != mBuffers[1].start)
                   fillSecond = true;
                // Keep the partially updated state consistent:
                mNValidBuffers = fillSecond ? 1 : 2;
             }
             else {
                // Request may fall between the clips of a track.
                // Invalidate all.  WaveTrack::Get() will return zeroes.
                mNValidBuffers = 0;
                fillSecond = false;
             }
          }
//          wxASSERT(!fillSecond || mNValidBuffers > 0);
          if (fillSecond) {
             mNValidBuffers = 1;
             const auto end0 = mBuffers[0].end();
             if (end > end0) {
                const auto start1 = mPTrack->GetBlockStart(end0);
                if (start1 == end0) {
                   const auto len1 = mPTrack->GetBestBlockSize(start1);
//                   wxASSERT(len1 <= mBufferSize);
                   if (!mPTrack->Get(samplePtr(mBuffers[1].data.get()), floatSample, start1, len1, fillZero, mayThrow))
                      return 0;
                   mBuffers[1].start = start1;
                   mBuffers[1].len = len1;
                   mNValidBuffers = 2;
                }
             }
          }
//          wxASSERT(mNValidBuffers < 2 || mBuffers[0].end() == mBuffers[1].start);

          samplePtr buffer = 0;
          auto remaining = len;

          // Possibly get an initial portion that is uncached

          // This may be negative
          const auto initLen =
             mNValidBuffers < 1 ? sampleCount( len )
                : std::min(sampleCount( len ), mBuffers[0].start - start);

          if (initLen > 0) {
             // This might be fetching zeroes between clips
             mOverlapBuffer.Resize(len, format);
             // initLen is not more than len:
             auto sinitLen = initLen.as_size_t();
             if (!mPTrack->Get(mOverlapBuffer.ptr(), format, start, sinitLen,
                               fillZero, mayThrow))
                return 0;
//             wxASSERT( sinitLen <= remaining );
             remaining -= sinitLen;
             start += initLen;
             buffer = mOverlapBuffer.ptr() + sinitLen * SAMPLE_SIZE(format);
          }

          // Now satisfy the request from the buffers
          for (int ii = 0; ii < mNValidBuffers && remaining > 0; ++ii) {
             const auto starti = start - mBuffers[ii].start;
             // Treatment of initLen above establishes this loop invariant,
             // and statements below preserve it:
//             wxASSERT(starti >= 0);

             // This may be negative
             const auto leni =
                std::min( sampleCount( remaining ), mBuffers[ii].len - starti );
             if (initLen <= 0 && leni == len) {
                // All is contiguous already.  We can completely avoid copying
                // leni is nonnegative, therefore start falls within mBuffers[ii],
                // so starti is bounded between 0 and buffer length
                return samplePtr(mBuffers[ii].data.get() + starti.as_size_t() );
             }
             else if (leni > 0) {
                // leni is nonnegative, therefore start falls within mBuffers[ii]
                // But we can't satisfy all from one buffer, so copy
                if (buffer == 0) {
                   mOverlapBuffer.Resize(len, format);
                   buffer = mOverlapBuffer.ptr();
                }
                // leni is positive and not more than remaining
                const size_t size = sizeof(float) * leni.as_size_t();
                // starti is less than mBuffers[ii].len and nonnegative
                memcpy(buffer, mBuffers[ii].data.get() + starti.as_size_t(), size);
//                wxASSERT( leni <= remaining );
                remaining -= leni.as_size_t();
                start += leni;
                buffer += size;
             }
          }

          if (remaining > 0) {
             // Very big request!
             // Fall back to direct fetch
             if (buffer == 0) {
                mOverlapBuffer.Resize(len, format);
                buffer = mOverlapBuffer.ptr();
             }
             if (!mPTrack->Get(buffer, format, start, remaining, fillZero, mayThrow))
                return 0;
          }

          return mOverlapBuffer.ptr();
       }

       // Cache works only for float format.
       mOverlapBuffer.Resize(len, format);
       if (mPTrack->Get(mOverlapBuffer.ptr(), format, start, len, fillZero, mayThrow))
          return mOverlapBuffer.ptr();
       else
          return 0;
    }

    void WaveTrackCache::Free()
    {
       mBuffers[0].Free();
       mBuffers[1].Free();
       mOverlapBuffer.Free();
       mNValidBuffers = 0;
    }

    void WaveTrack::GetEnvelopeValues(double *buffer, size_t bufferLen,
                                      double t0) const
    {
       // The output buffer corresponds to an unbroken span of time which the callers expect
       // to be fully valid.  As clips are processed below, the output buffer is updated with
       // envelope values from any portion of a clip, start, end, middle, or none at all.
       // Since this does not guarantee that the entire buffer is filled with values we need
       // to initialize the entire buffer to a default value.
       //
       // This does mean that, in the cases where a usable clip is located, the buffer value will
       // be set twice.  Unfortunately, there is no easy way around this since the clips are not
       // stored in increasing time order.  If they were, we could just track the time as the
       // buffer is filled.
       for (decltype(bufferLen) i = 0; i < bufferLen; i++)
       {
          buffer[i] = 1.0;
       }

       double startTime = t0;
       auto tstep = 1.0 / mRate;
       double endTime = t0 + tstep * bufferLen;
       for (const auto &clip: mClips)
       {
          // IF clip intersects startTime..endTime THEN...
          auto dClipStartTime = clip->GetStartTime();
          auto dClipEndTime = clip->GetEndTime();
          if ((dClipStartTime < endTime) && (dClipEndTime > startTime))
          {
             auto rbuf = buffer;
             auto rlen = bufferLen;
             auto rt0 = t0;

             if (rt0 < dClipStartTime)
             {
                // This is not more than the number of samples in
                // (endTime - startTime) which is bufferLen:
                auto nDiff = (sampleCount)floor((dClipStartTime - rt0) * mRate + 0.5);
                auto snDiff = nDiff.as_size_t();
                rbuf += snDiff;
//                wxASSERT(snDiff <= rlen);
                rlen -= snDiff;
                rt0 = dClipStartTime;
             }

             if (rt0 + rlen*tstep > dClipEndTime)
             {
                auto nClipLen = clip->GetEndSample() - clip->GetStartSample();

                if (nClipLen <= 0) // Testing for bug 641, this problem is consistently '== 0', but doesn't hurt to check <.
                   return;

                // This check prevents problem cited in http://bugzilla.audacityteam.org/show_bug.cgi?id=528#c11,
                // Gale's cross_fade_out project, which was already corrupted by bug 528.
                // This conditional prevents the previous write past the buffer end, in clip->GetEnvelope() call.
                // Never increase rlen here.
                // PRL bug 827:  rewrote it again
                rlen = limitSampleBufferSize( rlen, nClipLen );
                rlen = std::min(rlen, size_t(floor(0.5 + (dClipEndTime - rt0) / tstep)));
             }
             // Samples are obtained for the purpose of rendering a wave track,
             // so quantize time
             clip->GetEnvelope()->GetValues(rbuf, rlen, rt0, tstep);
          }
       }
    }

    float WaveTrack::GetChannelGain(int channel) const
    {
       float left = 1.0;
       float right = 1.0;

       if (mPan < 0)
          right = (mPan + 1.0);
       else if (mPan > 0)
          left = 1.0 - mPan;

       if ((channel%2) == 0)
          return left*mGain;
       else
          return right*mGain;
    }
}
