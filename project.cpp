#include "project.h"
#include "ui_project.h"
#include "commandmanager.h"
#include "importraw.h"
#include "track.h"
#include "WaveTrack.h"
#include "panelwrapper.h"
#include "pluginmanager.h"
#include "EffectManager.h"
#include "export.h"

#include <QVBoxLayout>
#include <QFileDialog>

namespace Renfeng {
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

      mTags = std::make_shared<Tags>();

      mTracks = TrackList::Create();

      mDirManager = std::make_shared<DirManager>();

      mTrackFactory.reset(new TrackFactory{ mDirManager, &mViewInfo });
      //        QVBoxLayout *vLayout = new QVBoxLayout;
      //        QHBoxLayout *hLayout = new QHBoxLayout;
      //        setLayout(vLayout);
      //        vLayout->addLayout(hLayout);
      //
      //        mMainFrame = new FrameWrapper(this);
      //        hLayout->addWidget(mMainFrame);
      //
      //        QVBoxLayout *v1Layout = new QVBoxLayout;
      //        mMainFrame->setLayout(v1Layout);

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
      amplifyAct->setObjectName(QString("Amplify"));
      QAction *noiseRedAct = effectMenu->addAction(QString("Noise Reduction"));
      noiseRedAct->setObjectName(QString("Noise Reduction"));

      QMenu *fileMenu = ui->menuBar->addMenu(QString("File"));
      QMenu *importMenu = fileMenu->addMenu(QString("Import"));
      QMenu *exportMenu = fileMenu->addMenu(QString("Export"));
      QAction *rawAct = importMenu->addAction(QString("Raw Data"));
      QAction *audioAct = exportMenu->addAction(QString("Export Audio"));

      // EffectMenu
      connect(amplifyAct, SIGNAL(triggered()), this, SLOT(menuClicked()));
      connect(noiseRedAct, SIGNAL(triggered()), this, SLOT(menuClicked()));

      // FileMenu
      connect(rawAct, SIGNAL(triggered()), this, SLOT(fileClicked()));
      connect(audioAct, SIGNAL(triggered()), this, SLOT(exportClicked()));
  }

  void AudioProject::menuClicked() {
      doEffect();
  }

  void AudioProject::fileClicked() {
      onImportRaw();
  }

  void AudioProject::exportClicked() {
      onExportAudio();
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

  void AudioProject::onExportAudio() {
      doExport("");
  }

  std::vector< std::shared_ptr< Track > >
  AudioProject::addImportedTracks(const QString &fileName,
                                  TrackHolders &&newTracks)
  {
      std::vector< std::shared_ptr< Track > > results;


      bool initiallyEmpty = mTracks->empty();
      double newRate = 0;
      int i = -1;

      // Must add all tracks first (before using Track::IsLeader)
      for (auto &group : newTracks) {
              if (group.empty()) {
                      //             wxASSERT(false);
                      continue;
                  }
              auto first = group.begin()->get();
              auto nChannels = group.size();
              for (auto &uNewTrack : group) {
                      auto newTrack = mTracks->Add(std::move(uNewTrack));
                      if (newRate == 0)
                          newRate = dynamic_cast<WaveTrack*>(newTrack)->GetRate();
                      newTrack->SetSelected(true);
                  }
              mTracks->GroupChannels(*first, nChannels);
          }
      newTracks.clear();

      mRate = newRate;

      return results;
  }

  void AudioProject::doEffect() {
      QString pluginID;
      QObject *obj = this->sender();
      if (obj->objectName() == QString("Amplify")) {
              pluginID = QString("Effect_Audacity_Audacity_Amplify_Built-in Effect: Amplify");
          } else {
              pluginID = QString("");
          }
      auto tracks = GetTracks();
      auto rate = GetRate();
      //     这里和原始代码有些出入 自己改了一下
      auto &selectedRegion = GetSelection();
      auto maxEnd = tracks->GetEndTime();
      selectedRegion.setT1(maxEnd);

      const PluginDescriptor *plug = PluginManager::get().getPlugin(pluginID);
      if (!plug)
          return;
      EffectType type = plug->getEffectType();

      auto nTracksOriginally = getTrackCount();

      //        int count = 0;
      //        bool clean = true;
      //        for (auto t : tracks->Selected< const WaveTrack >()) {
      //            if (t->GetEndTime() != 0.0)
      //                clean = false;
      //            count++;
      //        }

      EffectManager & em = EffectManager::Get();
      bool success = em.DoEffect(pluginID, this, rate,
                                 tracks, mTrackFactory.get(), &selectedRegion,
                                 true);
  }

  void AudioProject::doExport(const QString& format) {
      auto tracks = GetTracks();

      Exporter e;

      double t0 = 0.0;
      double t1 = tracks->GetEndTime();

      e.SetDefaultFormat(format);
      e.Process(this, false, t0, t1);
  }

  const Tags *AudioProject::GetTags()
  {
      return mTags.get();
  }
}
