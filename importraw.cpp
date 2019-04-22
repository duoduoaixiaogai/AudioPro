#include "importraw.h"
#include "track.h"
#include "sndfile.h"
#include "formatclassifier.h"

#include <QWidget>

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
        }
    }
}
