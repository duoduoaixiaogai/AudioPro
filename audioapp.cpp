#include "audioapp.h"
#include "pluginmanager.h"
#include "project.h"
#include "DirManager.h"

namespace RF {
    bool AudioApp::init() {
        DirManager::SetTempDir(QString("C://Users//Administrator//AppData//Local//Audacity//SessionData"));

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
