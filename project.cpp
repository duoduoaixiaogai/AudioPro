#include "project.h"
#include "ui_project.h"
#include <memoryx.h>
#include "panelwrapper.h"
#include "trackpanel.h"

#include <QVBoxLayout>

namespace RF {
    AProjectArray gAudioProjects;

    static AudioProject *gActiveProject;
    static void setActiveProject(AudioProject *project);

    void setActiveProject(AudioProject *project) {
        gActiveProject = project;

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
        QMainWindow(parent),
        ui(new Ui::AudioProject)
    {
        ui->setupUi(this);

        mTracks = TrackList::create();

        QVBoxLayout *vLayout = new QVBoxLayout;
        setLayout(vLayout);

        mMainFrame = new FrameWrapper(this);

        mTrackPanel = new TrackPanel();
    }

    AudioProject::~AudioProject()
    {
        delete ui;
    }

}
