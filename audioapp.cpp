#include "audioapp.h"
#include "pluginmanager.h"
#include "project.h"
#include "DirManager.h"

#include <QCoreApplication>

namespace Renfeng {
    bool AudioApp::init() {
        DirManager::SetTempDir(QCoreApplication::applicationDirPath());
        //DirManager::SetTempDir(QString("C://Users//49085//AppData//Local//AudioPro//SessionData"));
        //DirManager::SetTempDir(QString("C://Users//Administrator//AppData//Local//AudioPro//SessionData"));

        PluginManager::get().initialize();

        AudioProject *project;
//        {
//            initAudioIO();
//        }
        {
            project = createNewAudioProject();
        }
        return true;
    }
}
