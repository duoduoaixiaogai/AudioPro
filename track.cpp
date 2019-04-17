#include "track.h"

namespace RF {
    TrackList::TrackList() {

    }

    TrackList::~TrackList() {

    }

    std::shared_ptr<TrackList> TrackList::create() {
        std::shared_ptr<TrackList> result{new TrackList{}};
        result->mSelf = result;
        return result;
    }
}
