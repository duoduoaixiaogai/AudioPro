#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H

#include "types.h"

#include <unordered_map>
#include <map>

class QMainWindow;

namespace RF {


    class TrackList;
    class TrackFactory;
    class SelectedRegion;
    class Effect;
    class AudacityCommand;

    using EffectMap = std::map<QString, Effect *>;
    using AudacityCommandMap = std::map<QString, AudacityCommand *>;
    using EffectOwnerMap = std::map< QString, std::shared_ptr<Effect> >;
//    using EffectMap = std::unordered_map<QString, Effect *>;
//    using AudacityCommandMap = std::unordered_map<QString, AudacityCommand *>;
//    using EffectOwnerMap = std::unordered_map< QString, std::shared_ptr<Effect> >;


    class EffectManager
    {
    public:

        static EffectManager & Get();

    public:
        EffectManager();
        virtual ~EffectManager();

        bool DoEffect(const PluginID & ID,
                         ::QMainWindow *parent,
                         double projectRate,
                         TrackList *list,
                         TrackFactory *factory,
                         SelectedRegion *selectedRegion,
                         bool shouldPrompt = true);
    private:
        Effect *GetEffect(const PluginID & ID);
    private:
        EffectMap mEffects;
        AudacityCommandMap mCommands;
        EffectOwnerMap mHostEffects;
    };
}

#endif // EFFECTMANAGER_H
