#ifndef PROJECT_H
#define PROJECT_H

#include "types.h"
#include "DirManager.h"
#include "ViewInfo.h"
#include "importraw.h"

#include <QMainWindow>

#include <memory>

QT_BEGIN_NAMESPACE
class QFrame;
QT_END_NAMESPACE

namespace Ui {
    class AudioProject;
}

namespace RF {

    class TrackPanel;
    class CommandManager;
    class TrackFactory;
    class TrackList;


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
        ViewInfo mViewInfo;
        std::vector< std::shared_ptr<Track> >
           addImportedTracks(const QString &fileName,
                             TrackHolders &&newTracks);
        void SelectNone();
        TrackList *GetTracks() { return mTracks.get(); }
    private:
        void createMenus();
        void onImportRaw();

    private slots:
        void menuClicked();
        void fileClicked();
    private:
        Ui::AudioProject *ui;

        std::shared_ptr<TrackList> mTracks;
        TrackPanel *mTrackPanel{};
        QFrame *mMainFrame;
        std::unique_ptr<CommandManager> mCommandManager;
        std::unique_ptr<TrackFactory> mTrackFactory{};
        sampleFormat mDefaultFormat;
        double mRate;
        std::shared_ptr<DirManager> mDirManager;
    };

    AudioProject* GetActiveProject();
    AudioProject* createNewAudioProject();


    using AProjectHolder = std::shared_ptr<AudioProject>;
    using AProjectArray = std::vector<AProjectHolder>;

    extern AProjectArray gAudioProjects;
}

#endif // PROJECT_H
