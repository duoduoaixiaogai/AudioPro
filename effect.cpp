#include "effect.h"
#include "track.h"

namespace RF {

    Effect::Effect() {
        mClient = nullptr;
    }

    bool Effect::LoadFactoryDefaults() {
        return true;
    }

    EffectType Effect::getType() {
        return  EffectTypeNone;
    }

    ComponentInterfaceSymbol Effect::getFamilyId() {
        //        return ComponentInterfaceSymbol(QString(""));
        if (mClient)
        {
            return mClient->getFamilyId();
        }

        // Unusually, the internal and visible strings differ for the built-in
        // effect family.
        return { QString("Audacity"), QString("Built-in") };
    }

    bool Effect::isInteractive() {
        return true;
    }

    bool Effect::isDefault() {
        return true;
    }

    bool Effect::isLegacy() {
        if (mClient)
           {
              return false;
           }

           return true;
    }

    bool Effect::supportsRealtime() {
        return true;
    }

    bool Effect::supportsAutomation() {
        return true;
    }

    QString Effect::getPath() {
        //        return QString("");
        if (mClient)
        {
            return mClient->getPath();
        }

        return BUILTIN_EFFECT_PREFIX + getSymbol().internal();
    }

    ComponentInterfaceSymbol Effect::getSymbol() {
        return ComponentInterfaceSymbol(QString(""));
    }

    ComponentInterfaceSymbol Effect::getVendor() {
        //        return ComponentInterfaceSymbol(QString(""));
        if (mClient)
        {
            return mClient->getVendor();
        }

        return QString("Audacity");
    }

    QString Effect::getVersion() {
        return QString("");
    }

    QString Effect::getDescription() {
        return QString("");
    }

    bool Effect::SetHost(EffectHostInterface *host)
    {
       if (mClient)
       {
          return mClient->SetHost(host);
       }

       return true;
    }

    bool Effect::Startup(EffectClientInterface *client)
    {
       // Let destructor know we need to be shutdown
       mClient = client;

       // Set host so client startup can use our services
       if (!SetHost(this))
       {
          // Bail if the client startup fails
          mClient = nullptr;
          return false;
       }

       mNumAudioIn = GetAudioInCount();
       mNumAudioOut = GetAudioOutCount();

       return true;
    }

    unsigned Effect::GetAudioInCount()
    {
       if (mClient)
       {
          return mClient->GetAudioInCount();
       }

       return 0;
    }

    unsigned Effect::GetAudioOutCount()
    {
       if (mClient)
       {
          return mClient->GetAudioOutCount();
       }

       return 0;
    }

    bool Effect::DoEffect(::QMainWindow *parent,
                          double projectRate,
                          TrackList *list,
                          TrackFactory *factory,
                          SelectedRegion *selectedRegion,
                          bool shouldPrompt)
    {

            return true;
    }
}
