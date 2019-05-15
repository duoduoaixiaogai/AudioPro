#ifndef IMPORTRAW_H
#define IMPORTRAW_H

#include <vector>
#include <memory>

class QWidget;
class QString;

namespace Renfeng {

    class WaveTrack;
    class TrackFactory;

    using NewChannelGroup = std::vector<std::unique_ptr<WaveTrack> >;
    using TrackHolders = std::vector<NewChannelGroup>;

    void importRaw(QWidget *parent, const QString &fileName,
                   TrackFactory *trackFactory, TrackHolders &outTracks);
}

#endif // IMPORTRAW_H
