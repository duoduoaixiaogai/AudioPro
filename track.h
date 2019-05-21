#ifndef TRACK_H
#define TRACK_H

#include "commontrackpanelcell.h"
#include "SampleFormat.h"
#include "DirManager.h"
#include "ViewInfo.h"
#include "mix.h"

#include <QSize>

namespace Renfeng {

    class WaveTrack;
    class TrackList;

    using ListOfTracks = std::list< std::shared_ptr< Track > >;

    using TrackNodePointer =
    std::pair< ListOfTracks::iterator, ListOfTracks* >;

    inline bool operator == (const TrackNodePointer &a, const TrackNodePointer &b)
    { return a.second == b.second && a.first == b.first; }

    inline bool operator != (const TrackNodePointer &a, const TrackNodePointer &b)
    { return !(a == b); }

    enum class TrackKind
    {
        None,
        Wave,
#if defined(USE_MIDI)
        Note,
#endif
        Label,
        Time,
        Audio,
        Playable,
        All
    };

    constexpr bool CompatibleTrackKinds( TrackKind desired, TrackKind actual )
    {
        return
                (desired == actual)
                ||
                (desired == TrackKind::All)
                ||
                (desired == TrackKind::Audio    && actual == TrackKind::Wave)
        #ifdef USE_MIDI
                ||
                (desired == TrackKind::Audio    && actual == TrackKind::Note)
        #endif
                ||
                (desired == TrackKind::Playable && actual == TrackKind::Wave)
        #ifdef EXPERIMENTAL_MIDI_OUT
                ||
                (desired == TrackKind::Playable && actual == TrackKind::Note)
        #endif
                ;
    }

    namespace TrackTyper {
        template<typename, TrackKind> struct Pair;
        using List = std::tuple<
        Pair<Track,         TrackKind::All>,
        //         Pair<AudioTrack,    TrackKind::Audio>,
        //         Pair<PlayableTrack, TrackKind::Playable>,
        //         Pair<LabelTrack,    TrackKind::Label>,
        //         Pair<NoteTrack,     TrackKind::Note>,
        //         Pair<TimeTrack,     TrackKind::Time>,
        Pair<WaveTrack,     TrackKind::Wave>
        // New classes can be added easily to this list
        >;
        template<typename...> struct Lookup;
        template<typename TrackType, TrackKind Here, typename... Rest>
        struct Lookup< TrackType, std::tuple< Pair<TrackType, Here>, Rest... > > {
            static constexpr TrackKind value() {
                return Here;
            }
        };
        template<typename TrackType, typename NotHere, typename... Rest>
        struct Lookup< TrackType, std::tuple< NotHere, Rest... > > {
            static constexpr TrackKind value() {
                return Lookup< TrackType, std::tuple< Rest... > >::value();
            }
        };
    };

    template<typename TrackType> constexpr TrackKind track_kind ()
    {
        using namespace TrackTyper;
        return Lookup< typename std::remove_const<TrackType>::type, List >::value();
    }




    class Track : public CommonTrackPanelCell, public XMLTagHandler, public std::enable_shared_from_this<Track>
    {
        friend class TrackList;

    public:
        Track(const std::shared_ptr<DirManager> &mDirManager);
        virtual ~ Track();
        mutable QSize vrulerSize;
        enum ChannelType
        {
            LeftChannel = 0,
            RightChannel = 1,
            MonoChannel = 2
        };
        enum : unsigned { DefaultHeight = 150 };
        bool IsSelected() const;
        bool GetSelected() const { return mSelected; }
        virtual double GetEndTime() const = 0;
        virtual void SetSelected(bool s);
        bool IsLeader() const;
        bool IsSelectedLeader() const;
        bool IsSelectedOrSyncLockSelected() const;
        using Holder = std::shared_ptr<Track>;
        virtual Holder Duplicate() const = 0;
        template < typename R = void >
        using Continuation = std::function< R() >;
        using Fallthrough = Continuation<>;
        virtual double GetStartTime() const = 0;
        bool Any() const;
        virtual ChannelType GetChannel() const { return mChannel;}
        //        TrackId GetId() const { return mId; }

        template< typename R = void, typename ...Functions >
        R TypeSwitch(const Functions &...functions)
        {
            using WaveExecutor =
            Executor< R, WaveTrack,  Functions... >;
            //              using NoteExecutor =
            //                 Executor< R, NoteTrack,  Functions... >;
            //              using LabelExecutor =
            //                 Executor< R, LabelTrack, Functions... >;
            //              using TimeExecutor =
            //                 Executor< R, TimeTrack,  Functions... >;
            using DefaultExecutor =
            Executor< R, Track >;
            enum { All = sizeof...( functions ) };

            //          static_assert(
            //                      (1u << All) - 1u ==
            //                      (WaveExecutor::SetUsed /*|
            //                                  NoteExecutor::SetUsed |
            //                                  LabelExecutor::SetUsed |
            //                                  TimeExecutor::SetUsed*/),
            //                      "Uncallable case in Track::TypeSwitch"
            //                      );

            switch (GetKind()) {
                case TrackKind::Wave:
                    return WaveExecutor{} (this,  functions...);
#if defined(USE_MIDI)
                case TrackKind::Note:
                    return NoteExecutor{} (this,  functions...);
#endif
                    //                 case TrackKind::Label:
                    //                    return LabelExecutor{}(this, functions...);
                    //                 case TrackKind::Time:
                    //                    return TimeExecutor{} (this,  functions...);
                default:
                    return DefaultExecutor{} (this);
                }
        }

        virtual double GetOffset() const = 0;

        template<typename Subclass = Track>
           inline std::shared_ptr<Subclass> SharedPointer()
           {
              // shared_from_this is injected into class scope by base class
              // std::enable_shared_from_this<Track>
              return std::static_pointer_cast<Subclass>( shared_from_this() );
           }

           template<typename Subclass = const Track>
           inline auto SharedPointer() const -> typename
              std::enable_if<
                 std::is_const<Subclass>::value, std::shared_ptr<Subclass>
              >::type
           {
              // shared_from_this is injected into class scope by base class
              // std::enable_shared_from_this<Track>
              return std::static_pointer_cast<Subclass>( shared_from_this() );
           }

           // Static overloads of SharedPointer for when the pointer may be null
           template<typename Subclass = Track>
           static inline std::shared_ptr<Subclass> SharedPointer( Track *pTrack )
           { return pTrack ? pTrack->SharedPointer<Subclass>() : nullptr; }

           template<typename Subclass = const Track>
           static inline std::shared_ptr<Subclass> SharedPointer( const Track *pTrack )
           { return pTrack ? pTrack->SharedPointer<Subclass>() : nullptr; }
    private:
        // Variadic template specialized below
        template< typename ...Params >
        struct Executor;

        // This specialization grounds the recursion.
        template< typename R, typename ConcreteType >
        struct Executor< R, ConcreteType >
        {
            enum : unsigned { SetUsed = 0 };
            // No functions matched, so do nothing.
            R operator () (const void *) { return R{}; }
        };

        // And another specialization is needed for void return.
        template< typename ConcreteType >
        struct Executor< void, ConcreteType >
        {
            enum : unsigned { SetUsed = 0 };
            // No functions matched, so do nothing.
            void operator () (const void *) { }
        };

        // This struct groups some helpers needed to define the recursive cases of
        // Executor.
        struct Dispatcher {
            // This implements the specialization of Executor
            // for the first recursive case.
            template< typename R, typename ConcreteType,
                      typename Function, typename ...Functions >
            struct inapplicable
            {
                using Tail = Executor< R, ConcreteType, Functions... >;
                enum : unsigned { SetUsed = Tail::SetUsed << 1 };

                // Ignore the first, inapplicable function and try others.
                R operator ()
                (const Track *pTrack,
                 const Function &, const Functions &...functions)
                { return Tail{}( pTrack, functions... ); }
            };

            // This implements the specialization of Executor
            // for the second recursive case.
            template< typename R, typename BaseClass, typename ConcreteType,
                      typename Function, typename ...Functions >
            struct applicable1
            {
                enum : unsigned { SetUsed = 1u };

                // Ignore the remaining functions and call the first only.
                R operator ()
                (const Track *pTrack,
                 const Function &function, const Functions &...)
                { return function( (BaseClass *)pTrack ); }
            };

            // This implements the specialization of Executor
            // for the third recursive case.
            template< typename R, typename BaseClass, typename ConcreteType,
                      typename Function, typename ...Functions >
            struct applicable2
            {
                using Tail = Executor< R, ConcreteType, Functions... >;
                enum : unsigned { SetUsed = (Tail::SetUsed << 1) | 1u };

                // Call the first function, which may request dispatch to the further
                // functions by invoking a continuation.
                R operator ()
                (const Track *pTrack, const Function &function,
                 const Functions &...functions)
                {
                    auto continuation = Continuation<R>{ [&] {
                            return Tail{}( pTrack, functions... );
                        } };
                    return function( (BaseClass *)pTrack, continuation );
                }
            };

            // This variadic template chooses among the implementations above.
            template< typename ... > struct Switch;

            // Ground the recursion.
            template< typename R, typename ConcreteType >
            struct Switch< R, ConcreteType >
            {
                // No BaseClass of ConcreteType is acceptable to Function.
                template< typename Function, typename ...Functions >
                static auto test()
                -> inapplicable< R, ConcreteType, Function, Functions... >;
            };

            // Recursive case.
            template< typename R, typename ConcreteType,
                      typename BaseClass, typename ...BaseClasses >
            struct Switch< R, ConcreteType, BaseClass, BaseClasses... >
            {
                using Retry = Switch< R, ConcreteType, BaseClasses... >;

                // If ConcreteType is not compatible with BaseClass, or if
                // Function does not accept BaseClass, try other BaseClasses.
                template< typename Function, typename ...Functions >
                static auto test( const void * )
                -> decltype( Retry::template test< Function, Functions... >() );

                // If BaseClass is a base of ConcreteType and Function can take it,
                // then overload resolution chooses this.
                // If not, then the sfinae rule makes this overload unavailable.
                template< typename Function, typename ...Functions >
                static auto test( std::true_type * )
                -> decltype(
                        (void) std::declval<Function>()
                        ( (BaseClass*)nullptr ),
                        applicable1< R, BaseClass, ConcreteType,
                        Function, Functions... >{}
                        );

                // If BaseClass is a base of ConcreteType and Function can take it,
                // with a second argument for a continuation,
                // then overload resolution chooses this.
                // If not, then the sfinae rule makes this overload unavailable.
                template< typename Function, typename ...Functions >
                static auto test( std::true_type * )
                -> decltype(
                        (void) std::declval<Function>()
                        ( (BaseClass*)nullptr,
                          std::declval< Continuation<R> >() ),
                        applicable2< R, BaseClass, ConcreteType,
                        Function, Functions... >{}
                        );

                static constexpr bool Compatible = CompatibleTrackKinds(
                            track_kind<BaseClass>(), track_kind<ConcreteType>() );
                template< typename Function, typename ...Functions >
                static auto test()
                -> decltype(
                        test< Function, Functions... >(
                            (std::integral_constant<bool, Compatible>*)nullptr) );
            };
        };

        // This specialization is the recursive case for non-const tracks.
        template< typename R, typename ConcreteType,
                  typename Function, typename ...Functions >
        struct Executor< R, ConcreteType, Function, Functions... >
                : decltype(
                Dispatcher::Switch< R, ConcreteType,
                Track, /*AudioTrack, PlayableTrack,*/
                WaveTrack/*, LabelTrack, TimeTrack,
                                       NoteTrack*/ >
                ::template test<Function, Functions... >())
        {};

        // This specialization is the recursive case for const tracks.
        template< typename R, typename ConcreteType,
                  typename Function, typename ...Functions >
        struct Executor< R, const ConcreteType, Function, Functions... >
                : decltype(
                Dispatcher::Switch< R, ConcreteType,
                const Track, /*const AudioTrack, const PlayableTrack,*/
                const WaveTrack/*, const LabelTrack, const TimeTrack,
                                       const NoteTrack*/ >
                ::template test<Function, Functions... >())
        {};
    protected:
        mutable std::shared_ptr<DirManager> mDirManager;
        double              mOffset;
        bool           mLinked;
        int            mY;
        int            mHeight;
        int            mIndex;
        bool           mMinimized;
        ChannelType         mChannel;
        std::weak_ptr<TrackList> mList;
        TrackNodePointer mNode{};
    private:
        TrackNodePointer GetNode() const;
        bool           mSelected;
        virtual TrackKind GetKind() const { return TrackKind::None; }
        template<typename T>
        friend typename std::enable_if< std::is_pointer<T>::value, T >::type
        track_cast(Track *track);
        template<typename T>
        friend typename std::enable_if<
        std::is_pointer<T>::value &&
        std::is_const< typename std::remove_pointer< T >::type >::value,
        T
        >::type
        track_cast(const Track *track);
        Track *GetLink() const;
        void SetOwner
        (const std::weak_ptr<TrackList> &list, TrackNodePointer node);
        bool GetLinked  () const { return mLinked; }
        void SetChannel(ChannelType c) { mChannel = c; }
        std::shared_ptr<TrackList> GetOwner() const { return mList.lock(); }

    };

    class AudioTrack : public Track
    {
    public:
        AudioTrack(const std::shared_ptr<DirManager> &projDirManager)
            : Track{ projDirManager } {}
    };

    class PlayableTrack : public AudioTrack
    {
    public:
        PlayableTrack(const std::shared_ptr<DirManager> &projDirManager)
            : AudioTrack{ projDirManager } {}
        bool GetMute    () const { return mMute;     }
    protected:
       bool                mMute { false };
    };




    class TrackFactory
    {
    private:
        TrackFactory(const std::shared_ptr<DirManager> &dirManager, const ZoomInfo *zoomInfo):
            mDirManager(dirManager)
          , mZoomInfo(zoomInfo)
        {
        }

        const std::shared_ptr<DirManager> mDirManager;
        const ZoomInfo *const mZoomInfo;
        friend class AudioProject;

    public:
        // These methods are defined in WaveTrack.cpp, NoteTrack.cpp,
        // LabelTrack.cpp, and TimeTrack.cpp respectively
        std::unique_ptr<WaveTrack> NewWaveTrack(sampleFormat format = (sampleFormat)0,
                                                double rate = 0);
    };


    template<typename T>
    inline typename std::enable_if< std::is_pointer<T>::value, T >::type
    track_cast(Track *track)
    {
        using BareType = typename std::remove_pointer< T >::type;
        if (track &&
                CompatibleTrackKinds( track_kind<BareType>(), track->GetKind() ))
            return reinterpret_cast<T>(track);
        else
            return nullptr;
    }

    template<typename T>
    inline typename std::enable_if<
    std::is_pointer<T>::value &&
    std::is_const< typename std::remove_pointer< T >::type >::value,
    T
    >::type
    track_cast(const Track *track)
    {
        using BareType = typename std::remove_pointer< T >::type;
        if (track &&
                CompatibleTrackKinds( track_kind<BareType>(), track->GetKind() ))
            return reinterpret_cast<T>(track);
        else
            return nullptr;
    }

    template < typename TrackType > struct TrackIterRange;

    template <
            typename TrackType // Track or a subclass, maybe const-qualified
            > class TrackIter
            : public std::iterator<
            std::bidirectional_iterator_tag,
            TrackType *const,
            ptrdiff_t,
            // pointer is void to disable operator -> in the reverse_iterator...
            void,
            // ... because what operator * returns is really a value type,
            // so you can't take its address
            TrackType *const
            >
    {
    public:
        // Type of predicate taking pointer to const TrackType
        // TODO C++14:  simplify away ::type
        using FunctionType = std::function< bool(
        typename std::add_pointer<
        typename std::add_const<
        typename std::remove_pointer<
        TrackType
        >::type
        >::type
        >::type
        ) >;

        template<typename Predicate = FunctionType>
        TrackIter( TrackNodePointer begin, TrackNodePointer iter,
                   TrackNodePointer end, const Predicate &pred = {} )
            : mBegin( begin ), mIter( iter ), mEnd( end ), mPred( pred )
        {
            // Establish the class invariant
            if (this->mIter != this->mEnd && !this->valid())
                this->operator ++ ();
        }

        // Return an iterator that replaces the predicate, advancing to the first
        // position at or after the old position that satisfies the new predicate,
        // or to the end.
        template < typename Predicate2 >
        TrackIter Filter( const Predicate2 &pred2 ) const
        {
            return { this->mBegin, this->mIter, this->mEnd, pred2 };
        }

        // Return an iterator that refines the subclass (and not removing const),
        // advancing to the first position at or after the old position that
        // satisfies the type constraint, or to the end
        template < typename TrackType2 >
        auto Filter() const
        -> typename std::enable_if<
        std::is_base_of< TrackType, TrackType2 >::value &&
        (!std::is_const<TrackType>::value ||
         std::is_const<TrackType2>::value),
        TrackIter< TrackType2 >
        >::type
        {
            return { this->mBegin, this->mIter, this->mEnd, this->mPred };
        }

        const FunctionType &GetPredicate() const
        { return this->mPred; }

        // Unlike with STL iterators, this class gives well defined behavior when
        // you increment an end iterator: you get the same.
        TrackIter &operator ++ ()
        {
            // Maintain the class invariant
            if (this->mIter != this->mEnd) do
                ++this->mIter.first;
            while (this->mIter != this->mEnd && !this->valid() );
            return *this;
        }

        TrackIter operator ++ (int)
        {
            TrackIter result { *this };
            this-> operator ++ ();
            return result;
        }

        // Unlike with STL iterators, this class gives well defined behavior when
        // you decrement past the beginning of a range: you wrap and get an end
        // iterator.
        TrackIter &operator -- ()
        {
            // Maintain the class invariant
            do {
                    if (this->mIter == this->mBegin)
                        // Go circularly
                        this->mIter = this->mEnd;
                    else
                        --this->mIter.first;
                } while (this->mIter != this->mEnd && !this->valid() );
            return *this;
        }

        TrackIter operator -- (int)
        {
            TrackIter result { *this };
            this->operator -- ();
            return result;
        }

        // Unlike with STL iterators, this class gives well defined behavior when
        // you dereference an end iterator: you get a null pointer.
        TrackType *operator * () const
        {
            if (this->mIter == this->mEnd)
                return nullptr;
            else
                // Other methods guarantee that the cast is correct
                // (provided no operations on the TrackList invalidated
                // underlying iterators or replaced the tracks there)
                return static_cast< TrackType * >( &**this->mIter.first );
        }

        // This might be called operator + ,
        // but that might wrongly suggest constant time when the iterator is not
        // random access.
        TrackIter advance( long amount ) const
        {
            auto copy = *this;
            std::advance( copy, amount );
            return copy;
        }

        friend inline bool operator == (TrackIter a, TrackIter b)
        {
            // Assume the predicate is not stateful.  Just compare the iterators.
            return
                    a.mIter == b.mIter
                    // Assume this too:
                    // && a.mBegin == b.mBegin && a.mEnd == b.mEnd
                    ;
        }

        friend inline bool operator != (TrackIter a, TrackIter b)
        {
            return !(a == b);
        }

    private:
        bool valid() const
        {
            // assume mIter != mEnd
            const auto pTrack = track_cast< TrackType * >( &**this->mIter.first );
            if (!pTrack)
                return false;
            return !this->mPred || this->mPred( pTrack );
        }

        // This friendship is needed in TrackIterRange::StartingWith and
        // TrackIterRange::EndingAfter()
        friend TrackIterRange< TrackType >;

        // The class invariant is that mIter == mEnd, or else, mIter != mEnd and
        // **mIter is of the appropriate subclass and mPred(&**mIter) is true.
        TrackNodePointer mBegin, mIter, mEnd;
        FunctionType mPred;
    };

    template <
            typename TrackType // Track or a subclass, maybe const-qualified
            > struct TrackIterRange
            : public IteratorRange< TrackIter< TrackType > >
    {
        TrackIterRange
        ( const TrackIter< TrackType > &begin,
          const TrackIter< TrackType > &end )
            : IteratorRange< TrackIter< TrackType > >
              ( begin, end )
        {}

        // Conjoin the filter predicate with another predicate
        // Read + as "and"
        template< typename Predicate2 >
        TrackIterRange operator + ( const Predicate2 &pred2 ) const
        {
            const auto &pred1 = this->first.GetPredicate();
            using Function = typename TrackIter<TrackType>::FunctionType;
            const auto &newPred = pred1
                    ? Function{ [=] (typename Function::argument_type track) {
                    return pred1(track) && pred2(track);
                } }
                    : Function{ pred2 };
            return {
                    this->first.Filter( newPred ),
                            this->second.Filter( newPred )
                };
        }

        // Specify the added conjunct as a pointer to member function
        // Read + as "and"
        template< typename R, typename C >
        TrackIterRange operator + ( R ( C ::* pmf ) () const ) const
        {
            return this->operator + ( std::mem_fn( pmf ) );
        }

        // Conjoin the filter predicate with the negation of another predicate
        // Read - as "and not"
        template< typename Predicate2 >
        TrackIterRange operator - ( const Predicate2 &pred2 ) const
        {
            using ArgumentType =
            typename TrackIterRange::iterator::FunctionType::argument_type;
            auto neg = [=] (ArgumentType track) { return !pred2( track ); };
            return this->operator + ( neg );
        }

        // Specify the negated conjunct as a pointer to member function
        // Read - as "and not"
        template< typename R, typename C >
        TrackIterRange operator - ( R ( C ::* pmf ) () const ) const
        {
            return this->operator + ( std::not1( std::mem_fn( pmf ) ) );
        }

        template< typename TrackType2 >
        TrackIterRange< TrackType2 > Filter() const
        {
            return {
                    this-> first.template Filter< TrackType2 >(),
                            this->second.template Filter< TrackType2 >()
                };
        }

        TrackIterRange StartingWith( const Track *pTrack ) const
        {
            auto newBegin = this->find( pTrack );
            // More careful construction is needed so that the independent
            // increment and decrement of each iterator in the NEW pair
            // has the expected behavior at boundaries of the range
            return {
                    { newBegin.mIter, newBegin.mIter,    this->second.mEnd,
                                this->first.GetPredicate() },
                    { newBegin.mIter, this->second.mEnd, this->second.mEnd,
                                this->second.GetPredicate() }
                };
        }

        TrackIterRange EndingAfter( const Track *pTrack ) const
        {
            const auto newEnd = this->reversal().find( pTrack ).base();
            // More careful construction is needed so that the independent
            // increment and decrement of each iterator in the NEW pair
            // has the expected behavior at boundaries of the range
            return {
                    { this->first.mBegin, this->first.mIter, newEnd.mIter,
                                this->first.GetPredicate() },
                    { this->first.mBegin, newEnd.mIter,      newEnd.mIter,
                                this->second.GetPredicate() }
                };
        }

        // Exclude one given track
        TrackIterRange Excluding ( const TrackType *pExcluded ) const
        {
            return this->operator - (
                        [=](const Track *pTrack){ return pExcluded == pTrack; } );
        }

        // See Track::TypeSwitch
        template< typename ...Functions >
        void Visit(const Functions &...functions)
        {
            for (auto track : *this)
                track->TypeSwitch(functions...);
        }

        // See Track::TypeSwitch
        // Visit until flag is false, or no more tracks
        template< typename Flag, typename ...Functions >
        void VisitWhile(Flag &flag, const Functions &...functions)
        {
            if ( flag ) for (auto track : *this) {
                    track->TypeSwitch(functions...);
                    if (!flag)
                        break;
                }
        }
    };

    class TrackList final : /*public wxEvtHandler,*/ public ListOfTracks
    {
    public:
        friend class Track;
        static std::shared_ptr<TrackList> Create();
        template < typename TrackType = Track >
        auto Any()
        -> TrackIterRange< TrackType >
        {
            return Tracks< TrackType >();
        }

        template<typename TrackKind>
        Track* Add(std::unique_ptr<TrackKind> &&t) {
            Track *pTrack;
            push_back(ListOfTracks::value_type(pTrack = t.release()));

            auto n = getPrev( getEnd() );

            pTrack->SetOwner(mSelf, n);
            //   pTrack->SetId( TrackId{ ++sCounter } );
            //   RecalcPositions(n);
            //   AdditionEvent(n);
            return back().get();
        }
        void GroupChannels(
                Track &track, size_t groupSize, bool resetChannels = true );
        //        template < typename TrackType = Track >
        //        auto Selected()
        //        -> TrackIterRange< TrackType >
        //        {
        //            return Tracks< TrackType >( &Track::IsSelected );
        //        }
        template < typename TrackType = const Track >
        auto Selected() const
        -> typename std::enable_if< std::is_const<TrackType>::value,
        TrackIterRange< TrackType >
        >::type
        {
            return Tracks< TrackType >( &Track::IsSelected );
        }
        template < typename TrackType = Track >
        auto SelectedLeaders()
        -> TrackIterRange< TrackType >
        {
            return Tracks< TrackType >( &Track::IsSelectedLeader );
        }
        template<typename TrackKind>
        TrackKind *Add( const std::shared_ptr< TrackKind > &t )
        { return static_cast< TrackKind* >( DoAdd( t ) ); }
        template < typename TrackType = Track >
        auto Leaders()
        -> TrackIterRange< TrackType >
        {
            return Tracks< TrackType >( &Track::IsLeader );
        }
        template< typename TrackType >
        static auto Channels( TrackType *pTrack )
        -> TrackIterRange< TrackType >
        {
            return Channels_<TrackType>( pTrack->GetOwner()->FindLeader(pTrack) );
        }
        TrackIter< Track > FindLeader( Track *pTrack );
        template < typename TrackType = Track >
        auto Find(Track *pTrack)
        -> TrackIter< TrackType >
        {
            if (!pTrack || pTrack->GetOwner().get() != this)
                return EndIterator<TrackType>();
            else
                return MakeTrackIterator<TrackType>( pTrack->GetNode() );
        }

        // Turn a pointer into an iterator (constant time).
        template < typename TrackType = const Track >
        auto Find(const Track *pTrack) const
        -> typename std::enable_if< std::is_const<TrackType>::value,
        TrackIter< TrackType >
        >::type
        {
            if (!pTrack || pTrack->GetOwner().get() != this)
                return EndIterator<TrackType>();
            else
                return MakeTrackIterator<TrackType>( pTrack->GetNode() );
        }

        ListOfTracks::value_type Replace(
                Track * t, const ListOfTracks::value_type &with);

        TrackNodePointer Remove(Track *t);

        double GetEndTime() const;

        template < typename TrackType = const Track >
        auto Any() const
        -> typename std::enable_if< std::is_const<TrackType>::value,
        TrackIterRange< TrackType >
        >::type
        {
            return Tracks< TrackType >();
        }

        WaveTrackConstArray GetWaveTrackConstArray(bool selectionOnly, bool includeMuted = true) const;
    private:
        template < typename TrackType >
        TrackIter< TrackType >
        MakeTrackIterator( TrackNodePointer iter ) const
        {
            auto b = const_cast<TrackList*>(this)->getBegin();
            auto e = const_cast<TrackList*>(this)->getEnd();
            return { b, iter, e };
        }
        template < typename TrackType >
        TrackIter< TrackType >
        EndIterator() const
        {
            auto e = const_cast<TrackList*>(this)->getEnd();
            return { e, e, e };
        }
        Track *DoAdd(const std::shared_ptr<Track> &t);
        std::weak_ptr<TrackList> mSelf;
        template <
                typename TrackType = Track,
                typename Pred =
                typename TrackIterRange< TrackType >::iterator::FunctionType
                >
        auto Tracks( const Pred &pred = {} )
        -> TrackIterRange< TrackType >
        {
            auto b = getBegin(), e = getEnd();
            return { { b, b, e, pred }, { b, e, e, pred } };
        }
        template <
                typename TrackType = const Track,
                typename Pred =
                typename TrackIterRange< TrackType >::iterator::FunctionType
                >
        auto Tracks( const Pred &pred = {} ) const
        -> typename std::enable_if< std::is_const<TrackType>::value,
        TrackIterRange< TrackType >
        >::type
        {
            auto b = const_cast<TrackList*>(this)->getBegin();
            auto e = const_cast<TrackList*>(this)->getEnd();
            return { { b, b, e, pred }, { b, e, e, pred } };
        }
        bool isNull(TrackNodePointer p) const
        { return (p.second == this && p.first == ListOfTracks::end())
                    || (p.second == &mPendingUpdates && p.first == mPendingUpdates.end()); }
        TrackNodePointer getEnd() const
        { return { const_cast<TrackList*>(this)->ListOfTracks::end(),
                            const_cast<TrackList*>(this)}; }
        TrackNodePointer getBegin() const
        { return { const_cast<TrackList*>(this)->ListOfTracks::begin(),
                            const_cast<TrackList*>(this)}; }
        ListOfTracks mPendingUpdates;
        TrackNodePointer getNext(TrackNodePointer p) const
        {
            if ( isNull(p) )
                return p;
            auto q = p;
            ++q.first;
            return q;
        }
        TrackNodePointer getPrev(TrackNodePointer p) const
        {
            if (p == getBegin())
                return getEnd();
            else {
                    auto q = p;
                    --q.first;
                    return q;
                }
        }
        template< typename TrackType, typename InTrackType >
        static TrackIterRange< TrackType >
        Channels_( TrackIter< InTrackType > iter1 )
        {
            // Assume iterator filters leader tracks
            if (*iter1) {
                    return {
                            iter1.Filter( &Track::Any )
                                    .template Filter<TrackType>(),
                                    (++iter1).Filter( &Track::Any )
                                    .template Filter<TrackType>()
                        };
                }
            else
                // empty range
                return {
                        iter1.template Filter<TrackType>(),
                                iter1.template Filter<TrackType>()
                    };
        }
    };
}

#endif
