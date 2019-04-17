#ifndef OVERLAYPANEL_H
#define OVERLAYPANEL_H

#include "backedpanel.h"

namespace RF {

    class Overlay;

    class OverlayPanel : public BackedPanel {
    public:
        OverlayPanel(QWidget *parent = nullptr);
        void addOverlay(Overlay *pOverlay);
        bool removeOverlay(Overlay *pOverlay);
        void clearOverlays();
        void drawOverlays(bool repaint_all, QPaintDevice *pDC = nullptr);
    private:
        std::vector<Overlay*> mOverlays;
    };
}

#endif // OVERLAYPANEL_H
