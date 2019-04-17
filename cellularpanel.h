#ifndef CELLULARPANEL_H
#define CELLULARPANEL_H

#include "overlaypanel.h"

#include <functional>

namespace RF {

    class AudioProject;
    class TrackPanelNode;
    class TrackPanelCell;
    class TrackPanelGroup;
    class UIHandlePtr;

    class CellularPanel : public OverlayPanel {
    public:
        CellularPanel(QWidget *parent);
        ~CellularPanel() Q_DECL_OVERRIDE;
        virtual AudioProject* getProject() const = 0;
        virtual std::shared_ptr<TrackPanelNode> root() = 0;
        virtual TrackPanelCell* getFocusedCell() = 0;
        virtual void setFocusedCell() = 0;
        virtual void processUIHandleResult(
                TrackPanelCell *pClickedCell, TrackPanelCell *pLatestCell,
                unsigned refreshResult) = 0;
        virtual void updateStatusMessage(const QString &) = 0;
        virtual bool takesFocus() const = 0;
    public:
        struct Visitor {
            virtual ~Visitor();
            virtual void visitCell(const QRect &rect, TrackPanelCell &cell);
            virtual void beginGroup(const QRect &rect, TrackPanelGroup &group);
            virtual void endGroup(const QRect &rect, TrackPanelGroup &group);
        };

        void visit(Visitor &visitor);

        using SimpleCellVisitor =
            std::function<void(const QRect &rect, TrackPanelCell &cell) >;
        void visitCells(const SimpleCellVisitor &visitor);
        using SimpleNodeVisitor =
            std::function<void(const QRect &rect, TrackPanelNode &node) >;
        void visitPreorder(const SimpleNodeVisitor &visitor);
        void visitPostorder(const SimpleNodeVisitor &visitor);

        struct FoundCell {
            std::shared_ptr<TrackPanelCell> pCell;
            QRect rect;
        };

        FoundCell findCell(int mouseX, int mouseY);
        QRect findRect(const TrackPanelCell &cell);

        UIHandlePtr target();
        std::shared_ptr<TrackPanelCell> lastCell() const;
        bool isMouseCaptured();

        int mostRecentXCoord() const;
        void handleCursorForPresentMouseState(bool doHit = true);
    };
}

#endif // CELLULARPANEL_H
