#include "project.h"
#include "ui_project.h"
#include <memoryx.h>
#include "panelwrapper.h"
#include "trackpanel.h"
#include "commandmanager.h"
#include "importraw.h"

#include <QVBoxLayout>
#include <QFileDialog>
//#include <QMenu>
//#include <QMenuBar>

namespace RF {
    AProjectArray gAudioProjects;

    static AudioProject *gActiveProject;
    static void setActiveProject(AudioProject *project);

    void setActiveProject(AudioProject *project) {
        gActiveProject = project;

    }

    AudioProject* GetActiveProject() {
        return gActiveProject;
    }

    AudioProject* createNewAudioProject() {
        gAudioProjects.push_back(AProjectHolder{
                                     new AudioProject(
                                     nullptr, -1,
                                     QPoint(100, 100),
                                     QSize(600, 400)
                                     ),
                                     Destroyer<AudioProject>{}
                                 });
        const auto p = gAudioProjects.back().get();

        //gAudioIO->setListener(p);

        setActiveProject(p);

        p->show();

        return p;
    }

    AudioProject::AudioProject(QWidget *parent, int id, const QPoint &pos, const QSize &size) :
        QMainWindow(parent), mCommandManager(std::make_unique<CommandManager>),
        ui(new Ui::AudioProject)
    {
        ui->setupUi(this);

        mTracks = TrackList::create();

        //QVBoxLayout *vLayout = new QVBoxLayout;
        //setLayout(vLayout);

        //mMainFrame = new FrameWrapper(this);

        //mTrackPanel = new TrackPanel();
        createMenus();

    }

    AudioProject::~AudioProject()
    {
        delete ui;
    }

    void AudioProject::createMenus() {
        QMenu *effectMenu = ui->menuBar->addMenu(QString("Effect"));
        QAction *amplifyAct = effectMenu->addAction(QString("Amplify"));
        QAction *noiseRedAct = effectMenu->addAction(QString("Noise Reduction"));

        QMenu *fileMenu = ui->menuBar->addMenu(QString("File"));
        QMenu *importMenu = fileMenu->addMenu(QString("Import"));
        QAction *rawAct = importMenu->addAction(QString("Raw Data"));

        // EffectMenu
        connect(amplifyAct, &QAction::triggered, this, &AudioProject::menuClicked);
        connect(noiseRedAct, &QAction::triggered, this, &AudioProject::menuClicked);

        // FileMenu
        connect(rawAct, &QAction::trigger, this, &AudioProject::fileClicked);
    }

    void AudioProject::menuClicked(bool) {

    }

    void AudioProject::fileClicked(bool) {
        onImportRaw();
    }

    void AudioProject::onImportRaw() {
        QString fileName =
                QFileDialog::getOpenFileName(this, tr("Open File"),
                                             "c:",
                                             tr("PCM files (*.pcm)"));
        if (fileName.isNull()) {
            return;
        }

        TrackHolders newTracks;

        importRaw(this, fileName, mTrackFactory.get(), newTracks);

        if (newTracks.size() <= 0)
              return;

        addImportedTracks(fileName, std::move(newTracks));

    }

}
