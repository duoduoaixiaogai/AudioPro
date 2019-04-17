#ifndef PROJECT_H
#define PROJECT_H

#include "track.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QFrame;
QT_END_NAMESPACE

namespace Ui {
    class AudioProject;
}

namespace RF {

    class AudioProject;
    class TrackPanel;

    AudioProject* createNewAudioProject();

    using AProjectHolder = std::shared_ptr<AudioProject>;
    using AProjectArray = QVector<AProjectHolder>;

    extern AProjectArray gAudioProjects;

    class AudioProject : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit AudioProject(QWidget *parent, int id,
                              const QPoint &pos,
                              const QSize &size);
        ~AudioProject();

    private:
        Ui::AudioProject *ui;

        std::shared_ptr<TrackList> mTracks;
        TrackPanel *mTrackPanel{};
        QFrame *mMainFrame;
    };
}

#endif // PROJECT_H
