#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "types.h"
#include "moduleinterface.h"
#include "effectinterface.h"
#include "plugininterface.h"

#include <QMap>

namespace RF {
    typedef enum {
        PluginTypeNone = 0,
        PluginTypeStub = 1,
        PluginTypeEffect = 1 << 1,
        PluginTypeAudacityCommand = 1 << 2,
        PluginTypeExporter = 1 << 3,
        PluginTypeImporter = 1 << 4,
        PluginTypeModule = 1 << 5,
    } PluginType;

    class PluginDescriptor {
    public:
        PluginDescriptor();
        virtual ~PluginDescriptor();

        //        bool isInstantiated() const;
        //        ComponentInterface* getInstance();
        //        void setInstance(ComponentInterface *instance);
        //
        //        PluginType getPluginType() const;
        void setPluginType(PluginType type);
        //
        const QString& getID() const;
        //        const QString& getProviderID() const;
        const QString& getPath() const;
        //        const ComponentInterfaceSymbol& getSymbol() const;
        //
        //        QString getVendor() const;
        //
        //        bool isEnabled() const;
        //        bool isValid() const;
        //
        void setID(const PluginID &ID);
        void setProviderID(const PluginID& providerID);
        void setPath(const QString &path);
        void setSymbol(const ComponentInterfaceSymbol &symbol);
        //
        void setVersion(const QString &version);
        void setVendor(const QString &vendor);
        //
        void setEnabled(bool enable);
        void setValid(bool valid);
        //
        //        QString getEffectFamilyId() const;
        //
        EffectType getEffectType() const;
        //        bool isEffectDefault() const;
        //        bool isEffectInteractive() const;
        //        bool isEffectLegacy() const;
        //        bool isEffectRealtime() const;
        //        bool isEffectAutomatable() const;
        //
        void setEffectFamilyId(const QString &family);
        void setEffectType(EffectType type);
        void setEffectDefault(bool dflt);
        void setEffectInteractive(bool interactive);
        void setEffectLegacy(bool legacy);
        void setEffectRealtime(bool realtime);
        void setEffectAutomatable(bool automatable);
    private:
        PluginType mPluginType;
        QString mID;
        QString mPath;
        ComponentInterfaceSymbol mSymbol;
        QString mVersion;
        QString mVendor;
        QString mProviderID;
        bool mEnabled;
        bool mValid;

        //Effects
        QString mEffectFamily;
        EffectType mEffectType;
        bool mEffectInteractive;
        bool mEffectDefault;
        bool mEffectLegacy;
        bool mEffectRealtime;
        bool mEffectAutomatable;
    };


    typedef QMap<PluginID, PluginDescriptor> PluginMap;

    class PluginManager final : public PluginManagerInterface {
    public:
        const PluginID& registerPlugin(ModuleInterface *module) Q_DECL_OVERRIDE;
        const PluginID& registerPlugin(ModuleInterface *provider, ComponentInterface *command);
        const PluginID& registerPlugin(ModuleInterface *provider,
                                       EffectDefinitionInterface *effect,
                                       int type) Q_DECL_OVERRIDE;
        void initialize();
        void terminate();
        static PluginManager& get();
        static PluginID getID(ComponentInterface *module);
        static PluginID getID(EffectDefinitionInterface *effect);
        static QString getPluginTypeString(PluginType type);

        bool isPluginRegistered(const QString &path) Q_DECL_OVERRIDE;
        const PluginDescriptor *getPlugin(const PluginID & ID);

        ~PluginManager();
    private:
        PluginManager() = default;

        PluginDescriptor& createPlugin(const PluginID &id, ComponentInterface *ident, PluginType type);
    private:
        static std::unique_ptr<PluginManager> mInstance;
        PluginMap mPlugins;
    };
}

#endif // PLUGINMANAGER_H
