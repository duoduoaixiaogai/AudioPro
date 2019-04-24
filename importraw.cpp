#include "importraw.h"
#include "sndfile.h"
#include "formatclassifier.h"
#include "importrawdialog.h"
#include "FileFormats.h"
#include "WaveTrack.h"

#include <QWidget>
#include <QFile>

#include "track.h"

namespace RF {
    void importRaw(QWidget *parent, const QString &fileName,
                   TrackFactory *trackFactory, TrackHolders &outTracks) {
        outTracks.clear();
        int encoding = 0;
        sampleFormat format;
        sf_count_t offset = 0;
        double rate = 44100;
        double percent = 100.0;
        TrackHolders results;

        {
            SF_INFO sndInfo;
            unsigned numChannels = 0;

            try {
                FormatClassifier theClassifier(fileName.toStdString().data());
                encoding = theClassifier.GetResultFormatLibSndfile();
                numChannels = theClassifier.GetResultChannels();
                offset = 0;
            } catch (...) {
                encoding = 0;
            }

            if (encoding <= 0) {
                // Unable to guess.  Use mono, 16-bit samples with CPU endianness
                // as the default.
                encoding = SF_FORMAT_RAW | SF_ENDIAN_CPU | SF_FORMAT_PCM_16;
                numChannels = 1;
                offset = 0;
            }

            numChannels = std::max(1u, numChannels);

            ImportRawDialog dlog(encoding, numChannels, static_cast<int>(offset), rate, parent);
            dlog.show();
            if (!dlog.result()) {
                return;
            }

            encoding = dlog.mEncoding;
            numChannels = dlog.mChannels;
            rate = dlog.mRate;
            offset = static_cast<sf_count_t>(dlog.mOffset);
            percent = dlog.mPercent;

            memset(&sndInfo, 0, sizeof(SF_INFO));
            sndInfo.samplerate = static_cast<int>(rate);
            sndInfo.channels = static_cast<int>(numChannels);
            sndInfo.format = encoding | SF_FORMAT_RAW;

            QFile f(fileName);
            SFFile sndFile;

            if (f.open(QIODevice::ReadOnly)) {
                sndFile.reset(SFCall<SNDFILE*>(sf_open_fd, f.handle(), SFM_READ, &sndInfo, false));
            }

            if (!sndFile){
                char str[1000];
                sf_error_str(static_cast<SNDFILE *>(nullptr), str, 1000);
                //wxPrintf("%s\n", str);

                //throw FileException{ FileException::Cause::Open, fileName };
            }

            {
                int result = sf_command(sndFile.get(), SFC_SET_RAW_START_OFFSET, &offset, sizeof(offset));
                if (result != 0) {
                    char str[1000];
                    sf_error_str(sndFile.get(), str, 1000);
                    //wxPrintf("%s\n", str);

                    //throw FileException{ FileException::Cause::Read, fileName };
                }
            }
            SFCall<sf_count_t>(sf_seek, sndFile.get(), 0, SEEK_SET);
            auto totalFrames =
                    // fraction of a sf_count_t value
                    static_cast<sampleCount>(sndInfo.frames * percent / 100.0);

            if (format != floatSample &&
                    sf_subtype_more_than_16_bits(encoding))
                format = floatSample;

            results.resize(1);
            auto &channels = results[0];
            channels.resize(numChannels);

            {
                // iter not used outside this scope.
                auto iter = channels.begin();
                for (decltype(numChannels) c = 0; c < numChannels; ++iter, ++c) {
                    *iter = trackFactory->NewWaveTrack(format, rate);
                }
            }
            const auto firstChannel = channels.begin()->get();
            auto maxBlockSize = firstChannel->GetMaxBlockSize();
            SampleBuffer srcbuffer(maxBlockSize * numChannels, format);
            SampleBuffer buffer(maxBlockSize, format);

            decltype(totalFrames) framescompleted = 0;
            if (totalFrames < 0) {
                //static_assert(false, "text");
                totalFrames = 0;
            }

            size_t block;
            do {
                block =
                        limitSampleBufferSize( maxBlockSize, totalFrames - framescompleted );

                sf_count_t sf_result;
                if (format == int16Sample)
                    sf_result = SFCall<sf_count_t>(sf_readf_short, sndFile.get(), (short *)srcbuffer.ptr(), block);
                else
                    sf_result = SFCall<sf_count_t>(sf_readf_float, sndFile.get(), (float *)srcbuffer.ptr(), block);

                if (sf_result >= 0) {
                    block = sf_result;
                }
                else {
                    // This is not supposed to happen, sndfile.h says result is always
                    // a count, not an invalid value for error
                    //throw FileException{ FileException::Cause::Read, fileName };
                }

                if (block) {
                    auto iter = channels.begin();
                    for(decltype(numChannels) c = 0; c < numChannels; ++iter, ++c) {
                        if (format==int16Sample) {
                            for(decltype(block) j=0; j<block; j++)
                                ((short *)buffer.ptr())[j] =
                                    ((short *)srcbuffer.ptr())[numChannels*j+c];
                        }
                        else {
                            for(decltype(block) j=0; j<block; j++)
                                ((float *)buffer.ptr())[j] =
                                    ((float *)srcbuffer.ptr())[numChannels*j+c];
                        }

                        iter->get()->Append(buffer.ptr(), (format == int16Sample)?int16Sample:floatSample, block);
                    }
                    framescompleted += block;
                }

//                updateResult = progress.Update(
//                            framescompleted.as_long_long(),
//                            totalFrames.as_long_long()
//                            );
//                if (updateResult != ProgressResult::Success)
//                    break;

            } while (block > 0 && framescompleted < totalFrames);
        }

//        if (updateResult == ProgressResult::Failed || updateResult == ProgressResult::Cancelled)
//              throw UserException{};

           if (!results.empty() && !results[0].empty()) {
              for (const auto &channel : results[0])
                 channel->Flush();
              outTracks.swap(results);
           }
    }
}
