#include "EffectManager.h"
#include "track.h"
#include "effect.h"
#include "pluginmanager.h"

namespace RF {
    EffectManager & EffectManager::Get()
    {
       static EffectManager em;
       return em;
    }

    EffectManager::EffectManager()
    {

    }

    EffectManager::~EffectManager()
    {

    }

    bool EffectManager::DoEffect(const PluginID & ID,
                                 ::QMainWindow *parent,
                                 double projectRate,
                                 TrackList *list,
                                 TrackFactory *factory,
                                 SelectedRegion *selectedRegion,
                                 bool shouldPrompt /* = true */)

    {
       Effect *effect = GetEffect(ID);

       if (!effect)
       {
          return false;
       }

       bool res = effect->DoEffect(parent,
                                   projectRate,
                                   list,
                                   factory,
                                   selectedRegion,
                                   shouldPrompt);

       return res;
    }

    Effect *EffectManager::GetEffect(const PluginID & ID)
    {
       // Must have a "valid" ID
       if (ID.isEmpty())
       {
          return nullptr;
       }

       // If it is actually a command then refuse it (as an effect).
       if( mCommands.find( ID ) != mCommands.end() )
          return nullptr;

       // TODO: This is temporary and should be redone when all effects are converted
       if (mEffects.find(ID) == mEffects.end())
       {
          // This will instantiate the effect client if it hasn't already been done
          EffectDefinitionInterface *ident = dynamic_cast<EffectDefinitionInterface *>(PluginManager::get().getInstance(ID));
          if (ident && ident->isLegacy())
          {
             auto effect = dynamic_cast<Effect *>(ident);
             if (effect && effect->Startup(NULL))
             {
                mEffects[ID] = effect;
                return effect;
             }
          }

          auto effect = std::make_shared<Effect>(); // TODO: use make_unique and store in std::unordered_map
          if (effect)
          {
             EffectClientInterface *client = dynamic_cast<EffectClientInterface *>(ident);
             if (client && effect->Startup(client))
             {
                auto pEffect = effect.get();
                mEffects[ID] = pEffect;
                mHostEffects[ID] = std::move(effect);
                return pEffect;
             }
          }

          return nullptr;
       }

       return mEffects[ID];
    }

}
