#ifndef BACKEDPANEL_H
#define BACKEDPANEL_H

#include "panelwrapper.h"

namespace Renfeng {
    class BackedPanel : public FrameWrapper {
    public:
        BackedPanel(QWidget *parent = nullptr);
        ~BackedPanel();

        QPaintDevice& getBackingDC();
        QPaintDevice& getBackingDCForRepaint();
        void resizeBacking();
        void repairBitmap(QPaintDevice &dc, int x, int y, int width, int height);
        void displayBitmap(QPaintDevice &dc);
    protected:
//        void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    private:
        std::unique_ptr<QPixmap> mBacking;
        bool mResizeBacking{};
    };
}

#endif // BACKEDPANEL_H
