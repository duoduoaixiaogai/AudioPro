#ifndef TRACK_H
#define TRACK_H

#include "commontrackpanelcell.h"

#include <QList>

namespace RF {

    class Track;

    using ListOfTracks = std::list<std::shared_ptr<Track> >;

    class Track : public CommonTrackPanelCell {

    };

    class TrackList final : public ListOfTracks {
        TrackList();
    public:
        virtual ~TrackList();

        static std::shared_ptr<TrackList> create();

    private:
        std::weak_ptr<TrackList> mSelf;
    };

}

#endif // TRACK_H
