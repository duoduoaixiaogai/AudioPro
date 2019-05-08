#include "pluginmanager.h"
#include "modulemanager.h"

namespace RF {
    // Plugindescriptor
    PluginDescriptor::PluginDescriptor() {
        mPluginType = PluginTypeNone;
        mEnabled = false;
        mValid = false;
    }

    PluginDescriptor::~PluginDescriptor() {

    }

    void PluginDescriptor::setPluginType(PluginType type) {
        mPluginType = type;
    }

    const PluginID& PluginDescriptor::getID() const {
        return mID;
    }

    void PluginDescriptor::setID(const PluginID &ID) {
        mID = ID;
    }

    void PluginDescriptor::setProviderID(const PluginID &providerID) {
        mProviderID = providerID;
    }

    void PluginDescriptor::setPath(const QString &path) {
        mPath = path;
    }

    const QString& PluginDescriptor::getPath() const {
        return mPath;
    }

    void PluginDescriptor::setSymbol(const ComponentInterfaceSymbol &symbol) {
        mSymbol = symbol;
    }

    void PluginDescriptor::setVersion(const QString &version) {
        mVersion = version;
    }

    void PluginDescriptor::setVendor(const QString &vendor) {
        mVendor = vendor;
    }

    void PluginDescriptor::setEnabled(bool enable) {
        mEnabled = enable;
    }

    void PluginDescriptor::setValid(bool valid) {
        mValid = valid;
    }

    EffectType PluginDescriptor::getEffectType() const
    {
       return mEffectType;
    }

    void PluginDescriptor::setEffectFamilyId(const QString &family) {
        mEffectFamily = family;
    }

    void PluginDescriptor::setEffectType(EffectType type) {
        mEffectType = type;
    }

    void PluginDescriptor::setEffectInteractive(bool interactive) {
        mEffectInteractive = interactive;
    }

    void PluginDescriptor::setEffectDefault(bool dflt) {
        mEffectDefault = dflt;
    }

    void PluginDescriptor::setEffectRealtime(bool realtime) {
        mEffectRealtime = realtime;
    }

    void PluginDescriptor::setEffectAutomatable(bool automatable) {
        mEffectAutomatable = automatable;
    }

    std::unique_ptr<PluginManager> PluginManager::mInstance{};

    PluginManager::~PluginManager() {
        terminate();
    }

    const PluginID& PluginManagerInterface::defaultRegistrationCallback(ModuleInterface *provider,
                                                               ComponentInterface *pInterface) {
        EffectDefinitionInterface *pEInterface =
                dynamic_cast<EffectDefinitionInterface*>(pInterface);
        if (pEInterface) {
            return PluginManager::get().registerPlugin(provider, pEInterface, PluginTypeEffect);
        }
        ComponentInterface *pCInterface =
                dynamic_cast<ComponentInterface*>(pInterface);
        if (pCInterface) {
            return PluginManager::get().registerPlugin(provider, pCInterface);
        }

        static QString empty;
        return empty;
    }

    const PluginID& PluginManager::registerPlugin(ModuleInterface *module) {
        PluginDescriptor &plug = createPlugin(getID(module),
                                              module,
                                              PluginTypeModule);

        plug.setEnabled(true);
        plug.setValid(true);

        return plug.getID();
    }

    const PluginID& PluginManager::registerPlugin(ModuleInterface *provider, ComponentInterface *command) {
        PluginDescriptor &plug = createPlugin(getID(command), command, (PluginType)PluginTypeAudacityCommand);

        plug.setProviderID(PluginManager::getID(provider));

        plug.setEnabled(true);
        plug.setValid(true);

        return plug.getID();
    }


    const PluginID& PluginManager::registerPlugin(ModuleInterface *provider,
                                                  EffectDefinitionInterface *effect, int type) {
        PluginDescriptor &plug = createPlugin(getID(effect), effect, (PluginType)type);

        plug.setProviderID(PluginManager::getID(provider));

        plug.setEffectType(effect->getClassification());
        plug.setEffectFamilyId(effect->getFamilyId().internal());
        plug.setEffectInteractive(effect->isInteractive());
        plug.setEffectDefault(effect->isDefault());
        plug.setEffectRealtime(effect->supportsRealtime());
        plug.setEffectAutomatable(effect->supportsAutomation());

        plug.setEnabled(true);
        plug.setValid(true);

        return plug.getID();
    }

    PluginDescriptor& PluginManager::createPlugin(const PluginID &id,
                                                  ComponentInterface *ident,
                                                  PluginType type) {
        PluginDescriptor &plug = mPlugins[id];

        plug.setPluginType(type);

        plug.setID(id);
        plug.setPath(ident->getPath());
        plug.setSymbol(ident->getSymbol());
        plug.setVendor(ident->getVendor().internal());
        plug.setVersion(ident->getVersion());

        return plug;
    }

    void PluginManager::initialize() {
        ModuleManager::get().discoverProviders();

        const bool kFast = true;

    }

    void PluginManager::terminate() {

    }

    PluginManager& PluginManager::get() {
        if (!mInstance) {
            mInstance.reset(new PluginManager);
        }

        return *mInstance;
    }

    PluginID PluginManager::getID(ComponentInterface *module) {
        return QString("%1_%2_%3_%4_%5")
                .arg(getPluginTypeString(PluginTypeModule))
                .arg(QString(""))
                .arg(module->getVendor().internal())
                .arg(module->getSymbol().internal())
                .arg(module->getPath());
    }

    PluginID PluginManager::getID(EffectDefinitionInterface *effect)
    {
       return QString("%1_%2_%3_%4_%5")
                               .arg(getPluginTypeString(PluginTypeEffect))
                               .arg(effect->getFamilyId().internal())
                               .arg(effect->getVendor().internal())
                               .arg(effect->getSymbol().internal())
                               .arg(effect->getPath());
    }

    QString PluginManager::getPluginTypeString(PluginType type) {
        QString str;

        switch (type) {
            case PluginTypeNone:
                str = QString("Placeholder");
                break;
            case PluginTypeStub:
                str = QString("Stub");
                break;
            case PluginTypeEffect:
                str = QString("Effect");
                break;
            case PluginTypeAudacityCommand:
                str = QString("Generic");
                break;
            case PluginTypeExporter:
                str = QString("Exporter");
                break;
            case PluginTypeImporter:
                str = QString("Importer");
                break;
            case PluginTypeModule:
                str = QString("Module");
                break;
            default:
                break;
        }

        return str;
    }

    bool PluginManager::isPluginRegistered(const QString &path) {
        for (PluginMap::iterator iter = mPlugins.begin(); iter != mPlugins.end(); ++iter) {
            if (iter->getPath() == path) {
                return true;
            }
        }

        return false;
    }

    const PluginDescriptor *PluginManager::getPlugin(const PluginID & ID)
    {
       if (mPlugins.find(ID) == mPlugins.end())
       {
          return NULL;
       }

       return &mPlugins[ID];
    }
}
