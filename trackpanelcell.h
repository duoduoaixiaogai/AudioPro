#ifndef TRACKPANELCELL_H
#define TRACKPANELCELL_H

namespace RF {
    class TrackPanelNode {
    public:
        TrackPanelNode() = default;
        virtual ~TrackPanelNode() = 0;
    };

    class TrackPanelCell : public TrackPanelNode {
    public:
        virtual ~TrackPanelCell() = 0;

    };
}

#endif // TRACKPANELCELL_H
