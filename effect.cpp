#include "track.h"
#include "effect.h"
#include "memoryx.h"
#include "amplifyform.h"
#include "WaveTrack.h"

#include <QDialog>

namespace Renfeng {

  using t2bHash = std::unordered_map< void*, bool >;

#define QUANTIZED_TIME(time, rate) (floor(((double)(time) * (rate)) + 0.5) / (rate))

  EffectUIHost::EffectUIHost(QMainWindow *parent,
                             Effect *effect,
                             EffectUIClientInterface *client) {
      mEffect = effect;
      mClient = client;
  }

  Effect::Effect() {
      mClient = nullptr;
      mUIDialog = nullptr;
      mIsBatch = false;
  }

  bool Effect::LoadFactoryDefaults() {
      return true;
  }

  EffectType Effect::GetType() {
      return  EffectTypeNone;
  }

  ComponentInterfaceSymbol Effect::getFamilyId() {
      //        return ComponentInterfaceSymbol(QString(""));
      if (mClient)
          {
              return mClient->getFamilyId();
          }

      // Unusually, the internal and visible strings differ for the built-in
      // effect family.
      return { QString("Audacity"), QString("Built-in") };
  }

  bool Effect::isInteractive() {
      return true;
  }

  bool Effect::isDefault() {
      return true;
  }

  bool Effect::isLegacy() {
      if (mClient)
          {
              return false;
          }

      return true;
  }

  bool Effect::supportsRealtime() {
      return true;
  }

  bool Effect::supportsAutomation() {
      return true;
  }

  QString Effect::getPath() {
      //        return QString("");
      if (mClient)
          {
              return mClient->getPath();
          }

      return BUILTIN_EFFECT_PREFIX + getSymbol().internal();
  }

  ComponentInterfaceSymbol Effect::getSymbol() {
      return ComponentInterfaceSymbol(QString(""));
  }

  ComponentInterfaceSymbol Effect::getVendor() {
      //        return ComponentInterfaceSymbol(QString(""));
      if (mClient)
          {
              return mClient->getVendor();
          }

      return QString("Audacity");
  }

  QString Effect::getVersion() {
      return QString("");
  }

  QString Effect::getDescription() {
      return QString("");
  }

  bool Effect::SetHost(EffectHostInterface *host)
  {
      if (mClient)
          {
              return mClient->SetHost(host);
          }

      return true;
  }

  bool Effect::Startup(EffectClientInterface *client)
  {
      // Let destructor know we need to be shutdown
      mClient = client;

      // Set host so client startup can use our services
      if (!SetHost(this))
          {
              // Bail if the client startup fails
              mClient = nullptr;
              return false;
          }

      mNumAudioIn = GetAudioInCount();
      mNumAudioOut = GetAudioOutCount();

      return true;
  }

  unsigned Effect::GetAudioInCount()
  {
      if (mClient)
          {
              return mClient->GetAudioInCount();
          }

      return 0;
  }

  unsigned Effect::GetAudioOutCount()
  {
      if (mClient)
          {
              return mClient->GetAudioOutCount();
          }

      return 0;
  }

  bool Effect::DoEffect(::QMainWindow *parent,
                        double projectRate,
                        TrackList *list,
                        TrackFactory *factory,
                        SelectedRegion *selectedRegion,
                        bool shouldPrompt)
  {
      mOutputTracks.reset();

      mpSelectedRegion = selectedRegion;
      mFactory = factory;
      mProjectRate = projectRate;
      mTracks = list;

      CountWaveTracks();

      bool isSelection = false;

      mDuration = 0.0;

      mT0 = selectedRegion->t0();
      mT1 = selectedRegion->t1();

      if (mT1 > mT0)
          {
              // there is a selection: let's fit in there...
              // MJS: note that this is just for the TTC and is independent of the track rate
              // but we do need to make sure we have the right number of samples at the project rate
              double quantMT0 = QUANTIZED_TIME(mT0, mProjectRate);
              double quantMT1 = QUANTIZED_TIME(mT1, mProjectRate);
              mDuration = quantMT1 - quantMT0;
              isSelection = true;
              mT1 = mT0 + mDuration;
          }

      CountWaveTracks();

      if (!Init())
          {
              return false;
          }

      if (shouldPrompt && IsInteractive() && !PromptUser(parent))
          {
              return false;
          }

      bool returnVal = Process();

//      if (returnVal && (mT1 >= mT0 ))
//          {
//              selectedRegion->setTimes(mT0, mT1);
//          }

      return true;
  }

  void Effect::CountWaveTracks()
  {
      //    mNumTracks = mTracks->Selected< const WaveTrack >().size();
      //    mNumGroups = mTracks->SelectedLeaders< const WaveTrack >().size();
  }

  bool Effect::IsInteractive()
  {
      if (mClient)
          {
              return mClient->IsInteractive();
          }

      return true;
  }

  bool Effect::PromptUser(QMainWindow *parent)
  {
      return ShowInterface(parent, IsBatchProcessing());
  }

  bool Effect::IsBatchProcessing()
  {
      return mIsBatch;
  }

  bool Effect::ShowInterface(QMainWindow *parent, bool forceModal)
  {
      if (!IsInteractive())
          {
              return true;
          }

      if (mUIDialog)
          {
              if ( mUIDialog->close() )
                  mUIDialog = nullptr;
              return false;
          }

      if (mClient)
          {
              return mClient->ShowInterface(parent, forceModal);
          }

      // mUIDialog is null
      //       auto cleanup = valueRestorer( mUIDialog );

      mUIDialog = CreateUI(parent, this);
      if (!mUIDialog)
          {
              return false;
          }

      bool res = mUIDialog->exec() == QDialog::Rejected;
      //bool res = mUIDialog->exec() != 0;

      return res;
  }

  AmplifyDialog* Effect::CreateUI(QMainWindow *parent, EffectUIClientInterface *client)
  {
      Destroy_ptr<EffectUIHost> dlg
      { new EffectUIHost{ parent, this, client} };

      if (dlg->Initialize())
          {
              return dlg.release();
          }

      return nullptr;
  }

  bool EffectUIHost::Initialize()
  {
      //    AmplifyForm *form = new AmplifyForm(this);

      //    if (!mClient->PopulateUI(form))
      if (!mClient->PopulateUI(this))
          {
              return false;
          }

      return true;
  }

  bool Effect::PopulateUI(QWidget *parent)
  {
      PopulateOrExchange(parent);
      return true;
  }

  void Effect::PopulateOrExchange(QWidget *parent)
  {
      return;
  }

  bool Effect::Init()
  {
      return true;
  }

  bool Effect::Process()
  {
      CopyInputTracks(true);
      bool bGoodResult = true;

      // It's possible that the number of channels the effect expects changed based on
      // the parameters (the Audacity Reverb effect does when the stereo width is 0).
      mNumAudioIn = GetAudioInCount();
      mNumAudioOut = GetAudioOutCount();

      mPass = 1;
      if (true)
          {
              bGoodResult = ProcessPass();
              mPass = 2;
              //      if (bGoodResult && InitPass2())
              //      {
              //         bGoodResult = ProcessPass();
              //      }
          }

      ReplaceProcessedTracks(bGoodResult);

      return bGoodResult;
  }

  void Effect::CopyInputTracks(bool allSyncLockSelected)
  {
      // Reset map
      mIMap.clear();
      mOMap.clear();

      mOutputTracks = TrackList::Create();

      auto trackRange = mTracks->Any() +
              [&] (const Track *pTrack) {
              return allSyncLockSelected
                      ? pTrack->IsSelectedOrSyncLockSelected()
                      : track_cast<const WaveTrack*>( pTrack ) && pTrack->GetSelected();
          };

      t2bHash added;

      for (auto aTrack : trackRange)
          {
              Track *o = mOutputTracks->Add(aTrack->Duplicate());
              mIMap.push_back(aTrack);
              mOMap.push_back(o);
          }
  }

  bool Effect::ProcessPass()
  {
      bool bGoodResult = true;
      bool isGenerator = false;

      FloatBuffers inBuffer, outBuffer;
      ArrayOf<float *> inBufPos, outBufPos;

      ChannelName map[3];

      mBufferSize = 0;
      mBlockSize = 0;

      int count = 0;
      bool clear = false;

      const bool multichannel = mNumAudioIn > 1;
      auto range = multichannel
              ? mOutputTracks->Leaders()
              : mOutputTracks->Any();
      range.VisitWhile( bGoodResult,
                        [&](WaveTrack *left, const Track::Fallthrough &fallthrough) {
          if (!left->GetSelected())
              return fallthrough();

          sampleCount len;
          sampleCount leftStart;
          sampleCount rightStart = 0;

          if (!isGenerator)
              {
                  GetSamples(left, &leftStart, &len);
                  mSampleCnt = len;
              }
          else
              {
                  len = 0;
                  leftStart = 0;
                  mSampleCnt = left->TimeToLongSamples(mDuration);
              }

          mNumChannels = 0;
          WaveTrack *right{};

          // Iterate either over one track which could be any channel,
          // or if multichannel, then over all channels of left,
          // which is a leader.
          for (auto channel :
               TrackList::Channels(left).StartingWith(left)) {
                  if (channel->GetChannel() == Track::LeftChannel)
                      map[mNumChannels] = ChannelNameFrontLeft;
                  else if (channel->GetChannel() == Track::RightChannel)
                      map[mNumChannels] = ChannelNameFrontRight;
                  else
                      map[mNumChannels] = ChannelNameMono;

                  ++ mNumChannels;
                  map[mNumChannels] = ChannelNameEOL;

                  if (! multichannel)
                      break;

                  if (mNumChannels == 2) {
                          // TODO: more-than-two-channels
                          right = channel;
                          clear = false;
                          if (!isGenerator)
                              GetSamples(right, &rightStart, &len);

                          // Ignore other channels
                          break;
                      }
              }

          // Let the client know the sample rate
          SetSampleRate(left->GetRate());

          // Get the block size the client wants to use
          auto max = left->GetMaxBlockSize() * 2;
          mBlockSize = SetBlockSize(max);

          // Calculate the buffer size to be at least the max rounded up to the clients
          // selected block size.
          const auto prevBufferSize = mBufferSize;
          mBufferSize = ((max + (mBlockSize - 1)) / mBlockSize) * mBlockSize;

          // If the buffer size has changed, then (re)allocate the buffers
          if (prevBufferSize != mBufferSize)
              {
                  // Always create the number of input buffers the client expects even if we don't have
                  // the same number of channels.
                  inBufPos.reinit( mNumAudioIn );
                  inBuffer.reinit( mNumAudioIn, mBufferSize );

                  // We won't be using more than the first 2 buffers, so clear the rest (if any)
                  for (size_t i = 2; i < mNumAudioIn; i++)
                      {
                          for (size_t j = 0; j < mBufferSize; j++)
                              {
                                  inBuffer[i][j] = 0.0;
                              }
                      }

                  // Always create the number of output buffers the client expects even if we don't have
                  // the same number of channels.
                  outBufPos.reinit( mNumAudioOut );
                  // Output buffers get an extra mBlockSize worth to give extra room if
                  // the plugin adds latency
                  outBuffer.reinit( mNumAudioOut, mBufferSize + mBlockSize );
              }

          // (Re)Set the input buffer positions
          for (size_t i = 0; i < mNumAudioIn; i++)
              {
                  inBufPos[i] = inBuffer[i].get();
              }

          // (Re)Set the output buffer positions
          for (size_t i = 0; i < mNumAudioOut; i++)
              {
                  outBufPos[i] = outBuffer[i].get();
              }

          // Clear unused input buffers
          if (!right && !clear && mNumAudioIn > 1)
              {
                  for (size_t j = 0; j < mBufferSize; j++)
                      {
                          inBuffer[1][j] = 0.0;
                      }
                  clear = true;
              }

          // Go process the track(s)
          bGoodResult = ProcessTrack(
                      count, map, left, right, leftStart, rightStart, len,
                      inBuffer, outBuffer, inBufPos, outBufPos);
          if (!bGoodResult)
              return;

          count++;
      }/*,
      [&](Track *t) {
         if (t->IsSyncLockSelected())
            t->SyncLockAdjust(mT1, mT0 + mDuration);
      }*/
      );

      if (bGoodResult && GetType() == EffectTypeGenerate)
          {
              mT1 = mT0 + mDuration;
          }

      return bGoodResult;
  }

  void Effect::GetSamples(
          const WaveTrack *track, sampleCount *start, sampleCount *len)
  {
      double trackStart = track->GetStartTime();
      double trackEnd = track->GetEndTime();
      double t0 = mT0 < trackStart ? trackStart : mT0;
      double t1 = mT1 > trackEnd ? trackEnd : mT1;


      if (t1 > t0) {
              *start = track->TimeToLongSamples(t0);
              auto end = track->TimeToLongSamples(t1);
              *len = end - *start;
          }
      else {
              *start = 0;
              *len  = 0;
          }
  }

  void Effect::SetSampleRate(double rate)
  {
      if (mClient)
          {
              mClient->SetSampleRate(rate);
          }

      mSampleRate = rate;
  }

  size_t Effect::SetBlockSize(size_t maxBlockSize)
  {
      if (mClient)
          {
              return mClient->SetBlockSize(maxBlockSize);
          }

      mBlockSize = maxBlockSize;

      return mBlockSize;
  }

  bool Effect::ProcessTrack(int count,
                            ChannelNames map,
                            WaveTrack *left,
                            WaveTrack *right,
                            sampleCount leftStart,
                            sampleCount rightStart,
                            sampleCount len,
                            FloatBuffers &inBuffer,
                            FloatBuffers &outBuffer,
                            ArrayOf< float * > &inBufPos,
                            ArrayOf< float *> &outBufPos)
  {
      bool rc = true;

      // Give the plugin a chance to initialize
      if (!ProcessInitialize(len, map))
          {
              return false;
          }

      { // Start scope for cleanup
          auto cleanup = finally( [&] {
                  // Allow the plugin to cleanup
                  if (!ProcessFinalize())
                      {
                          // In case of non-exceptional flow of control, set rc
                          rc = false;
                      }
              } );

          // For each input block of samples, we pass it to the effect along with a
          // variable output location.  This output location is simply a pointer into a
          // much larger buffer.  This reduces the number of calls required to add the
          // samples to the output track.
          //
          // Upon return from the effect, the output samples are "moved to the left" by
          // the number of samples in the current latency setting, effectively removing any
          // delay introduced by the effect.
          //
          // At the same time the total number of delayed samples are gathered and when
          // there is no further input data to process, the loop continues to call the
          // effect with an empty input buffer until the effect has had a chance to
          // return all of the remaining delayed samples.
          auto inLeftPos = leftStart;
          auto inRightPos = rightStart;
          auto outLeftPos = leftStart;
          auto outRightPos = rightStart;

          auto inputRemaining = len;
          decltype(GetLatency()) curDelay = 0, delayRemaining = 0;
          decltype(mBlockSize) curBlockSize = 0;

          decltype(mBufferSize) inputBufferCnt = 0;
          decltype(mBufferSize) outputBufferCnt = 0;
          bool cleared = false;

          auto chans = std::min<unsigned>(mNumAudioOut, mNumChannels);

          std::shared_ptr<WaveTrack> genLeft, genRight;

          decltype(len) genLength = 0;
          bool isGenerator = GetType() == EffectTypeGenerate;
          bool isProcessor = GetType() == EffectTypeProcess;
          double genDur = 0;

          // Call the effect until we run out of input or delayed samples
          while (inputRemaining != 0 || delayRemaining != 0)
              {
                  // Still working on the input samples
                  if (inputRemaining != 0)
                      {
                          // Need to refill the input buffers
                          if (inputBufferCnt == 0)
                              {
                                  // Calculate the number of samples to get
                                  inputBufferCnt =
                                          limitSampleBufferSize( mBufferSize, inputRemaining );

                                  // Fill the input buffers
                                  left->Get((samplePtr) inBuffer[0].get(), floatSample, inLeftPos, inputBufferCnt);
                                  if (right)
                                      {
                                          right->Get((samplePtr) inBuffer[1].get(), floatSample, inRightPos, inputBufferCnt);
                                      }

                                  // Reset the input buffer positions
                                  for (size_t i = 0; i < mNumChannels; i++)
                                      {
                                          inBufPos[i] = inBuffer[i].get();
                                      }
                              }

                          // Calculate the number of samples to process
                          curBlockSize = mBlockSize;
                          if (curBlockSize > inputRemaining)
                              {
                                  // We've reached the last block...set current block size to what's left
                                  // inputRemaining is positive and bounded by a size_t
                                  curBlockSize = inputRemaining.as_size_t();
                                  inputRemaining = 0;

                                  // Clear the remainder of the buffers so that a full block can be passed
                                  // to the effect
                                  auto cnt = mBlockSize - curBlockSize;
                                  for (size_t i = 0; i < mNumChannels; i++)
                                      {
                                          for (decltype(cnt) j = 0 ; j < cnt; j++)
                                              {
                                                  inBufPos[i][j + curBlockSize] = 0.0;
                                              }
                                      }

                                  // Might be able to use up some of the delayed samples
                                  if (delayRemaining != 0)
                                      {
                                          // Don't use more than needed
                                          cnt = limitSampleBufferSize(cnt, delayRemaining);
                                          delayRemaining -= cnt;
                                          curBlockSize += cnt;
                                      }
                              }
                      }
                  // We've exhausted the input samples and are now working on the delay
                  else if (delayRemaining != 0)
                      {
                          // Calculate the number of samples to process
                          curBlockSize = limitSampleBufferSize( mBlockSize, delayRemaining );
                          delayRemaining -= curBlockSize;

                          // From this point on, we only want to feed zeros to the plugin
                          if (!cleared)
                              {
                                  // Reset the input buffer positions
                                  for (size_t i = 0; i < mNumChannels; i++)
                                      {
                                          inBufPos[i] = inBuffer[i].get();

                                          // And clear
                                          for (size_t j = 0; j < mBlockSize; j++)
                                              {
                                                  inBuffer[i][j] = 0.0;
                                              }
                                      }
                                  cleared = true;
                              }
                      }

                  // Finally call the plugin to process the block
                  decltype(curBlockSize) processed;
                  try
                  {
                      processed = ProcessBlock(inBufPos.get(), outBufPos.get(), curBlockSize);
                  }
                  //      catch( const AudacityException & WXUNUSED(e) )
                  //      {
                  //         // PRL: Bug 437:
                  //         // Pass this along to our application-level handler
                  //         throw;
                  //      }
                  catch(...)
                  {
                      // PRL:
                      // Exceptions for other reasons, maybe in third-party code...
                      // Continue treating them as we used to, but I wonder if these
                      // should now be treated the same way.
                      return false;
                  }
                  //      wxASSERT(processed == curBlockSize);
                  //      wxUnusedVar(processed);

                  // Bump to next input buffer position
                  if (inputRemaining != 0)
                      {
                          for (size_t i = 0; i < mNumChannels; i++)
                              {
                                  inBufPos[i] += curBlockSize;
                              }
                          inputRemaining -= curBlockSize;
                          inputBufferCnt -= curBlockSize;
                      }

                  // "ls" and "rs" serve as the input sample index for the left and
                  // right channels when processing the input samples.  If we flip
                  // over to processing delayed samples, they simply become counters
                  // for the progress display.
                  inLeftPos += curBlockSize;
                  inRightPos += curBlockSize;

                  // Get the current number of delayed samples and accumulate
                  if (isProcessor)
                      {
                          {
                              auto delay = GetLatency();
                              curDelay += delay;
                              delayRemaining += delay;
                          }

                          // If the plugin has delayed the output by more samples than our current
                          // block size, then we leave the output pointers alone.  This effectively
                          // removes those delayed samples from the output buffer.
                          if (curDelay >= curBlockSize)
                              {
                                  curDelay -= curBlockSize;
                                  curBlockSize = 0;
                              }
                          // We have some delayed samples, at the beginning of the output samples,
                          // so overlay them by shifting the remaining output samples.
                          else if (curDelay > 0)
                              {
                                  // curDelay is bounded by curBlockSize:
                                  auto delay = curDelay.as_size_t();
                                  curBlockSize -= delay;
                                  for (size_t i = 0; i < chans; i++)
                                      {
                                          memmove(outBufPos[i], outBufPos[i] + delay, sizeof(float) * curBlockSize);
                                      }
                                  curDelay = 0;
                              }
                      }

                  // Adjust the number of samples in the output buffers
                  outputBufferCnt += curBlockSize;

                  // Still have room in the output buffers
                  if (outputBufferCnt < mBufferSize)
                      {
                          // Bump to next output buffer position
                          for (size_t i = 0; i < chans; i++)
                              {
                                  outBufPos[i] += curBlockSize;
                              }
                      }
                  // Output buffers have filled
                  else
                      {
                          if (isProcessor)
                              {
                                  // Write them out
                                  left->Set((samplePtr) outBuffer[0].get(), floatSample, outLeftPos, outputBufferCnt);
                                  if (right)
                                      {
                                          if (chans >= 2)
                                              {
                                                  right->Set((samplePtr) outBuffer[1].get(), floatSample, outRightPos, outputBufferCnt);
                                              }
                                          else
                                              {
                                                  right->Set((samplePtr) outBuffer[0].get(), floatSample, outRightPos, outputBufferCnt);
                                              }
                                      }
                              }
                          else if (isGenerator)
                              {
                                  genLeft->Append((samplePtr) outBuffer[0].get(), floatSample, outputBufferCnt);
                                  if (genRight)
                                      {
                                          genRight->Append((samplePtr) outBuffer[1].get(), floatSample, outputBufferCnt);
                                      }
                              }

                          // Reset the output buffer positions
                          for (size_t i = 0; i < chans; i++)
                              {
                                  outBufPos[i] = outBuffer[i].get();
                              }

                          // Bump to the next track position
                          outLeftPos += outputBufferCnt;
                          outRightPos += outputBufferCnt;
                          outputBufferCnt = 0;
                      }

                  if (mNumChannels > 1)
                      {
                          //         if (TrackGroupProgress(count,
                          //               (inLeftPos - leftStart).as_double() /
                          //               (isGenerator ? genLength : len).as_double()))
                          //         {
                          //            rc = false;
                          //            break;
                          //         }
                      }
                  else
                      {
                          //         if (TrackProgress(count,
                          //               (inLeftPos - leftStart).as_double() /
                          //               (isGenerator ? genLength : len).as_double()))
                          //         {
                          //            rc = false;
                          //            break;
                          //         }
                      }
              }

          // Put any remaining output
          if (rc && outputBufferCnt)
              {
                  if (isProcessor)
                      {
                          left->Set((samplePtr) outBuffer[0].get(), floatSample, outLeftPos, outputBufferCnt);
                          if (right)
                              {
                                  if (chans >= 2)
                                      {
                                          right->Set((samplePtr) outBuffer[1].get(), floatSample, outRightPos, outputBufferCnt);
                                      }
                                  else
                                      {
                                          right->Set((samplePtr) outBuffer[0].get(), floatSample, outRightPos, outputBufferCnt);
                                      }
                              }
                      }
                  else if (isGenerator)
                      {
                          genLeft->Append((samplePtr) outBuffer[0].get(), floatSample, outputBufferCnt);
                          if (genRight)
                              {
                                  genRight->Append((samplePtr) outBuffer[1].get(), floatSample, outputBufferCnt);
                              }
                      }
              }

          //   if (rc && isGenerator)
          //   {
          //      AudacityProject *p = GetActiveProject();
          //
          //      // PRL:  this code was here and could not have been the right
          //      // intent, mixing time and sampleCount values:
          //      // StepTimeWarper warper(mT0 + genLength, genLength - (mT1 - mT0));
          //
          //      // This looks like what it should have been:
          //      // StepTimeWarper warper(mT0 + genDur, genDur - (mT1 - mT0));
          //      // But rather than fix it, I will just disable the use of it for now.
          //      // The purpose was to remap split lines inside the selected region when
          //      // a generator replaces it with sound of different duration.  But
          //      // the "correct" version might have the effect of mapping some splits too
          //      // far left, to before the selection.
          //      // In practice the wrong version probably did nothing most of the time,
          //      // because the cutoff time for the step time warper was 44100 times too
          //      // far from mT0.
          //
          //      // Transfer the data from the temporary tracks to the actual ones
          //      genLeft->Flush();
          //      // mT1 gives us the NEW selection. We want to replace up to GetSel1().
          //      auto &selectedRegion = p->GetViewInfo().selectedRegion;
          //      left->ClearAndPaste(mT0,
          //         selectedRegion.t1(), genLeft.get(), true, true,
          //         nullptr /* &warper */);
          //
          //      if (genRight)
          //      {
          //         genRight->Flush();
          //         right->ClearAndPaste(mT0, mT1, genRight.get(), true, true,
          //                              nullptr /* &warper */);
          //      }
          //   }

      } // End scope for cleanup
      return rc;
  }

  bool Effect::ProcessInitialize(sampleCount totalLen, ChannelNames chanMap)
  {
      if (mClient)
          {
              return mClient->ProcessInitialize(totalLen, chanMap);
          }

      return true;
  }

  bool Effect::ProcessFinalize()
  {
      if (mClient)
          {
              return mClient->ProcessFinalize();
          }

      return true;
  }

  sampleCount Effect::GetLatency()
  {
      if (mClient)
          {
              return mClient->GetLatency();
          }

      return 0;
  }

  size_t Effect::ProcessBlock(float **inBlock, float **outBlock, size_t blockLen)
  {
      if (mClient)
          {
              return mClient->ProcessBlock(inBlock, outBlock, blockLen);
          }

      return 0;
  }

  void Effect::ReplaceProcessedTracks(const bool bGoodResult)
  {
      //   if (!bGoodResult) {
      //      // Free resources, unless already freed.
      //
      //      // Processing failed or was cancelled so throw away the processed tracks.
      //      if ( mOutputTracks )
      //         mOutputTracks->Clear();
      //
      //      // Reset map
      //      mIMap.clear();
      //      mOMap.clear();
      //
      //      //TODO:undo the non-gui ODTask transfer
      //      return;
      //   }

      // Assume resources need to be freed.
      //   wxASSERT(mOutputTracks); // Make sure we at least did the CopyInputTracks().

      auto iterOut = mOutputTracks->ListOfTracks::begin(),
              iterEnd = mOutputTracks->ListOfTracks::end();

      size_t cnt = mOMap.size();
      size_t i = 0;

      for (; iterOut != iterEnd; ++i) {
              ListOfTracks::value_type o = *iterOut;
              // If tracks were removed from mOutputTracks, then there will be
              // tracks in the map that must be removed from mTracks.
              while (i < cnt && mOMap[i] != o.get()) {
                      const auto t = mIMap[i];
                      if (t) {
                              mTracks->Remove(t);
                          }
                      i++;
                  }
              //
              //      // This should never happen
              //      wxASSERT(i < cnt);

              // Remove the track from the output list...don't DELETE it
              iterOut = mOutputTracks->erase(iterOut);

              const auto  t = mIMap[i];
              if (t == NULL)
                  {
                      // This track is a NEW addition to output tracks; add it to mTracks
                      mTracks->Add( o );
                  }
              else
                  {
                      // Replace mTracks entry with the NEW track
                      mTracks->Replace(t, o);
                      //
                      //         // If the track is a wave track,
                      //         // Swap the wavecache track the ondemand task uses, since now the NEW
                      //         // one will be kept in the project
                      //         if (ODManager::IsInstanceCreated()) {
                      //            ODManager::Instance()->ReplaceWaveTrack( t, o.get() );
                      //         }
                  }
          }

      // If tracks were removed from mOutputTracks, then there may be tracks
      // left at the end of the map that must be removed from mTracks.
      //   while (i < cnt) {
      //      const auto t = mIMap[i];
      //      if (t) {
      //         mTracks->Remove(t);
      //      }
      //      i++;
      //   }
      //
      //   // Reset map
      mIMap.clear();
      mOMap.clear();
      //
      //   // Make sure we processed everything
      //   wxASSERT(mOutputTracks->empty());
      //
      //   // The output list is no longer needed
      mOutputTracks.reset();
      //   nEffectsDone++;
  }
}
