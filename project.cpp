#include "project.h"
#include "ui_project.h"
#include "commandmanager.h"
#include "importraw.h"
#include "track.h"
#include "WaveTrack.h"
#include "panelwrapper.h"

#include <QVBoxLayout>
#include <QFileDialog>

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
        QMainWindow(parent), /*(mCommandManager(std::make_unique<CommandManager>),*/
        mViewInfo(0.0, 1.0, ZoomInfo::GetDefaultZoom()),
        ui(new Ui::AudioProject)
    {
        ui->setupUi(this);

        mTracks = TrackList::Create();

        mDirManager = std::make_shared<DirManager>();

        mTrackFactory.reset(new TrackFactory{ mDirManager, &mViewInfo });
        QVBoxLayout *vLayout = new QVBoxLayout;
        QHBoxLayout *hLayout = new QHBoxLayout;
        setLayout(vLayout);
        vLayout->addLayout(hLayout);

        mMainFrame = new FrameWrapper(this);
        hLayout->addWidget(mMainFrame);

        QVBoxLayout *v1Layout = new QVBoxLayout;
        mMainFrame->setLayout(v1Layout);

        //mTrackPanel = new TrackPanel(mMainFrame, );
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
        connect(amplifyAct, SIGNAL(triggered()), this, SLOT(menuClicked()));
        connect(noiseRedAct, SIGNAL(triggered()), this, SLOT(menuClicked()));

        // FileMenu
        connect(rawAct, SIGNAL(triggered()), this, SLOT(fileClicked()));
    }

    void AudioProject::menuClicked() {

    }

    void AudioProject::fileClicked() {
        onImportRaw();
    }

    void AudioProject::onImportRaw() {
        QString fileName =
                QFileDialog::getOpenFileName(this, tr("Open File"),
                                             "c://Users//Administrator//Desktop",
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

    std::vector< std::shared_ptr< Track > >
    AudioProject::addImportedTracks(const QString &fileName,
                                       TrackHolders &&newTracks)
    {
       std::vector< std::shared_ptr< Track > > results;

       SelectNone();

       bool initiallyEmpty = mTracks->empty();
       double newRate = 0;
       wxString trackNameBase = fileName.AfterLast(wxFILE_SEP_PATH).BeforeLast('.');
       int i = -1;

       // Must add all tracks first (before using Track::IsLeader)
       for (auto &group : newTracks) {
          if (group.empty()) {
             wxASSERT(false);
             continue;
          }
          auto first = group.begin()->get();
          auto nChannels = group.size();
          for (auto &uNewTrack : group) {
             auto newTrack = mTracks->Add(std::move(uNewTrack));
             results.push_back(Track::Pointer(newTrack));
          }
          mTracks->GroupChannels(*first, nChannels);
       }
       newTracks.clear();

       // Now name them

       // Add numbers to track names only if there is more than one (mono or stereo)
       // track (not necessarily, more than one channel)
       const bool useSuffix =
          make_iterator_range( results.begin() + 1, results.end() )
             .any_of( []( decltype(*results.begin()) &pTrack )
                { return pTrack->IsLeader(); } );

       for (const auto &newTrack : results) {
          if ( newTrack->IsLeader() )
             // Count groups only
             ++i;

          newTrack->SetSelected(true);

          if ( useSuffix )
             newTrack->SetName(trackNameBase + wxString::Format(wxT(" %d" ), i + 1));
          else
             newTrack->SetName(trackNameBase);

          newTrack->TypeSwitch( [&](WaveTrack *wt) {
             if (newRate == 0)
                newRate = wt->GetRate();

             // Check if NEW track contains aliased blockfiles and if yes,
             // remember this to show a warning later
             if(WaveClip* clip = wt->GetClipByIndex(0)) {
                BlockArray &blocks = clip->GetSequence()->GetBlockArray();
                if (blocks.size())
                {
                   SeqBlock& block = blocks[0];
                   if (block.f->IsAlias())
                   {
                      mImportedDependencies = true;
                   }
                }
             }
          });
       }

       // Automatically assign rate of imported file to whole project,
       // if this is the first file that is imported
       if (initiallyEmpty && newRate > 0) {
          mRate = newRate;
          GetSelectionBar()->SetRate(mRate);
       }

       PushState(wxString::Format(_("Imported '%s'"), fileName),
                 _("Import"));

    #if defined(__WXGTK__)
       // See bug #1224
       // The track panel hasn't we been fully created, so the DoZoomFit() will not give
       // expected results due to a window width of zero.  Should be safe to yield here to
       // allow the creattion to complete.  If this becomes a problem, it "might" be possible
       // to queue a dummy event to trigger the DoZoomFit().
       wxEventLoopBase::GetActive()->YieldFor(wxEVT_CATEGORY_UI | wxEVT_CATEGORY_USER_INPUT);
    #endif

       if (initiallyEmpty && !IsProjectSaved() ) {
          wxString name = fileName.AfterLast(wxFILE_SEP_PATH).BeforeLast(wxT('.'));
          mFileName =::wxPathOnly(fileName) + wxFILE_SEP_PATH + name + wxT(".aup");
          mbLoadedFromAup = false;
          SetProjectTitle();
       }

       // Moved this call to higher levels to prevent flicker redrawing everything on each file.
       //   HandleResize();

       return results;
    }

    void AudioProject::SelectNone()
    {
       for (auto t : GetTracks()->Any())
          t->SetSelected(false);

       mTrackPanel->Refresh(false);
    }

}
