#ifndef TRACKPANELCELL_H
#define TRACKPANELCELL_H

namespace Renfeng {
    class TrackPanelNode
    {
    public:
        TrackPanelNode();
        virtual ~TrackPanelNode() = 0;
    };

    class TrackPanelCell : public TrackPanelNode
    {
    public:
        virtual ~TrackPanelCell () = 0;
    };
}

#endif
