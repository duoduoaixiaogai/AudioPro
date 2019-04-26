#include "backedpanel.h"

namespace RF {
    BackedPanel::BackedPanel(QWidget *parent)
        : FrameWrapper (parent)
        , mBacking{std::make_unique<QPixmap>(1, 1)} {}

    BackedPanel::~BackedPanel() {
        if (mBacking) {

        }
    }

    QPaintDevice& BackedPanel::getBackingDC() {
        return *mBacking;
    }

    QPaintDevice& BackedPanel::getBackingDCForRepaint() {
        if (mResizeBacking) {
            mResizeBacking = false;

            resizeBacking();
        }

        return *mBacking;
    }


    void BackedPanel::resizeBacking() {
        QSize sz = contentsRect().size();
        mBacking = std::make_unique<QPixmap>(std::max(sz.width(), 1),
                                             std::max(sz.height(), 1));
    }

    void BackedPanel::repairBitmap(QPaintDevice &dc, int x, int y, int width, int height) {
        dynamic_cast<QPixmap&>(dc) = mBacking->copy(x, y, width, height);
    }

    void BackedPanel::displayBitmap(QPaintDevice &dc) {
        if (mBacking) {
            repairBitmap(dc, 0, 0, mBacking->width(), mBacking->height());
        }
    }

//    void BackedPanel::resizeEvent(QResizeEvent *event) {
//        Q_UNUSED(event)
//
//        mResizeBacking = true;
//    }
}
