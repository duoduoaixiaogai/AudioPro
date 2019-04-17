#ifndef COMPONENTINTERFACE_H
#define COMPONENTINTERFACE_H

#include <QString>

namespace RF {
    class ComponentInterfaceSymbol {
    public:
        ComponentInterfaceSymbol() = default;
        ComponentInterfaceSymbol(const QString &msgid)
            : mInternal(msgid), mMsgid(msgid)
        {}
        ComponentInterfaceSymbol(const QString &internal, const QString &msgid)
            : mInternal(internal), mMsgid(internal.isEmpty() ? QString() : msgid)
        {}

        const QString& internal() const {return mInternal;}
        const QString& msgid() const {return mMsgid;}
        bool empty() const {return mInternal.isEmpty();}

        friend inline bool operator == (
                const ComponentInterfaceSymbol &a, const ComponentInterfaceSymbol &b)
        {return a.mInternal == b.mInternal;}

        friend inline bool operator != (
                const ComponentInterfaceSymbol &a, const ComponentInterfaceSymbol &b)
        {return !(a == b);}
    private:
        QString mInternal;
        QString mMsgid;
    };

    class ComponentInterface {
    public:
        virtual ~ComponentInterface() {}
        virtual QString getPath() = 0;
        virtual ComponentInterfaceSymbol getSymbol() = 0;
        virtual ComponentInterfaceSymbol getVendor() = 0;
        virtual QString getVersion() = 0;
        virtual QString getDescription() = 0;
    };
}

#endif // COMPONENTINTERFACE_H
