#ifndef PROJECT_H
#define PROJECT_H

#include "track.h"

#include <QMainWindow>

#include <memory>

QT_BEGIN_NAMESPACE
class QFrame;
QT_END_NAMESPACE

namespace Ui {
    class AudioProject;
}

namespace RF {

    class AudioProject;
    class TrackPanel;
    class CommandManager;
    class TrackFactory;

    AudioProject* GetActiveProject();
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
        CommandManager* getCommandManager() {
            return mCommandManager.get();
        }
        const CommandManager* getCommandManager() const {
            return mCommandManager.get();
        }
        sampleFormat GetDefaultFormat() { return mDefaultFormat; }
        double GetRate() const { return mRate; }
    private:
        void createMenus();
        void onImportRaw();

    private slots:
        void menuClicked(bool);
        void fileClicked(bool);
    private:
        Ui::AudioProject *ui;

        std::shared_ptr<TrackList> mTracks;
        TrackPanel *mTrackPanel{};
        QFrame *mMainFrame;
        std::unique_ptr<CommandManager> mCommandManager;
        std::unique_ptr<TrackFactory> mTrackFactory{};
        sampleFormat mDefaultFormat;
        double mRate;
    };
}

#endif // PROJECT_H
