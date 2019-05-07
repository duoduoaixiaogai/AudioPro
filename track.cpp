#include "track.h"

namespace RF {

    Track::Track(const std::shared_ptr<DirManager> &projDirManager)
        :  vrulerSize(36,0),
          mDirManager(projDirManager)
    {
        mSelected  = false;
        mLinked    = false;

        mY = 0;
        mHeight = DefaultHeight;
        mIndex = 0;

        mMinimized = false;

        mOffset = 0.0;

        mChannel = MonoChannel;
    }

    Track::~Track()
    {
    }

    std::shared_ptr<TrackList> TrackList::Create()
    {
        std::shared_ptr<TrackList> result{ new TrackList{} };
        result->mSelf = result;
        return result;
    }
}
