#include "audioapp.h"
#include "pluginmanager.h"
#include "project.h"

namespace RF {
    bool AudioApp::init() {
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
