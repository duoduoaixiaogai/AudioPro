#include "track.h"

namespace Renfeng {

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

    void Track::SetOwner
    (const std::weak_ptr<TrackList> &list, TrackNodePointer node)
    {
        // BUG: When using this function to clear an owner, we may need to clear
        // focussed track too.  Otherwise focus could remain on an invisible (or deleted) track.
        mList = list;
        mNode = node;
    }

//    template<typename TrackKind>
//    Track* TrackList::Add(std::unique_ptr<TrackKind> &&t)
//    {
//        Track *pTrack;
//        push_back(ListOfTracks::value_type(pTrack = t.release()));
//
//        auto n = getPrev( getEnd() );
//
//        pTrack->SetOwner(mSelf, n);
//        //   pTrack->SetId( TrackId{ ++sCounter } );
//        //   RecalcPositions(n);
//        //   AdditionEvent(n);
//        return back().get();
//    }

    void TrackList::GroupChannels(
            Track &track, size_t groupSize, bool resetChannels )
    {
        // If group size is more than two, for now only the first two channels
        // are grouped as stereo, and any others remain mono
        auto list = track.mList.lock();
        if ( groupSize > 0 && list.get() == this  ) {
            auto iter = track.mNode.first;
            auto after = iter;
            auto end = this->ListOfTracks::end();
            auto count = groupSize;
            for ( ; after != end && count; ++after, --count )
                ;

            if ( groupSize > 1 ) {
                const auto channel = *iter++;
                channel->SetChannel( Track::LeftChannel );
                (*iter++)->SetChannel( Track::RightChannel );
                while (iter != after)
                    (*iter++)->SetChannel( Track::MonoChannel );
            }
            return;
        }
    }

    bool Track::IsSelected() const
       { return GetSelected(); }

    void Track::SetSelected(bool s)
    {
       if (mSelected != s) {
          mSelected = s;
       }
    }

    bool Track::IsLeader() const
       { return !GetLink() || GetLinked(); }

    bool Track::IsSelectedLeader() const
       { return IsSelected() && IsLeader(); }

    bool Track::IsSelectedOrSyncLockSelected() const
       { return GetSelected() || false; }

    Track *Track::GetLink() const
    {
       auto pList = mList.lock();
       if (!pList)
          return nullptr;

       if (!pList->isNull(mNode)) {
          if (mLinked) {
             auto next = pList->getNext( mNode );
             if ( !pList->isNull( next ) )
                return next.first->get();
          }

          if (mNode.first != mNode.second->begin()) {
             auto prev = pList->getPrev( mNode );
             if ( !pList->isNull( prev ) ) {
                auto track = prev.first->get();
                if (track && track->GetLinked())
                   return track;
             }
          }
       }

       return nullptr;
    }

    Track *TrackList::DoAdd(const std::shared_ptr<Track> &t)
    {
       push_back(t);

//       auto n = getPrev( getEnd() );
//
//       t->SetOwner(shared_from_this(), n);
//       t->SetId( TrackId{ ++sCounter } );
//       RecalcPositions(n);
//       AdditionEvent(n);
       return back().get();
    }

    bool Track::Any() const
       { return true; }

    auto TrackList::FindLeader( Track *pTrack )
       -> TrackIter< Track >
    {
       auto iter = Find(pTrack);
       while( *iter && ! ( *iter )->IsLeader() )
          --iter;
       return iter.Filter( &Track::IsLeader );
    }

    TrackNodePointer Track::GetNode() const
    {
//       wxASSERT(mList.lock() == NULL || this == mNode.first->get());
       return mNode;
    }
}
