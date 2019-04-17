#include "overlaypanel.h"
#include "overlay.h"

namespace RF {
        OverlayPanel::OverlayPanel(QWidget *parent)
            :BackedPanel(parent){}
        void OverlayPanel::addOverlay(Overlay *pOverlay) {
            mOverlays.push_back(pOverlay);
        }
        bool OverlayPanel::removeOverlay(Overlay *pOverlay) {
            const size_t oldSize = mOverlays.size();
            mOverlays.erase(std::remove(mOverlays.begin(),
                                        mOverlays.end(),
                                        pOverlay), mOverlays.end());
            return oldSize != mOverlays.size();
        }
        void OverlayPanel::clearOverlays() {
            mOverlays.clear();
        }
        void OverlayPanel::drawOverlays(bool repaint_all, QPaintDevice *pDC) {
            size_t n_pairs = mOverlays.size();

            std::vector<std::pair<QRect, bool> > pairs;
            pairs.reserve(n_pairs);

            QSize size(dynamic_cast<QPixmap&>(getBackingDC()).size());
            for (const auto pOverlay : mOverlays) {
                pairs.push_back(pOverlay->getRectangle(size));
            }

            bool some_overlays_need_repainting = repaint_all;
            if (!repaint_all) {
                bool done;
                do {
                    done = true;
                    for (size_t ii = 0; ii < n_pairs; ++ii) {
                        some_overlays_need_repainting = pairs[ii].second;
                        for (size_t jj = ii + 1; jj < n_pairs; ++jj) {
                            if (pairs[ii].second != pairs[jj].second &&
                                    pairs[ii].first.intersects(pairs[jj].first)) {
                                done = false;
                                pairs[ii].second = pairs[jj].second = true;
                            }
                        }
                    }
                } while (!done);
            }

            if (!some_overlays_need_repainting) {
                return;
            }

            auto &dc = *pDC;

            bool done = true;
            auto it2 = pairs.begin();
            for (auto pOverlay : mOverlays) {
                if (repaint_all || it2->second) {
                    done = false;
                    pOverlay->erase(dc, getBackingDC());
                }
                ++it2;
            }

            if (!done) {
                it2 = pairs.begin();
                for (auto pOverlay : mOverlays) {
                    if (repaint_all || it2->second) {
                        pOverlay->draw(*this, dc);
                    }
                    ++it2;
                }
            }
        }
}

