#ifndef AMPLIFY_H
#define AMPLIFY_H

#include "effect.h"
#include "componentinterface.h"
#include "effectinterface.h"

namespace RF {
#define AMPLIFY_PLUGIN_SYMBOL ComponentInterfaceSymbol(QString("Amplify"))

    class EffectAmplify final : public Effect {
    public:
        EffectAmplify();
        virtual ~EffectAmplify();
        //        // ComponentInterface implementation
        ComponentInterfaceSymbol getSymbol() Q_DECL_OVERRIDE;
        //        QString getDescription() Q_DECL_OVERRIDE;
        //        QString ManualPage() Q_DECL_OVERRIDE;
        //        // EffectDefinitionInterface implementation
        //        EffectType getType() Q_DECL_OVERRIDE;
        //        // EffectClientInterface implementation
        //        unsigned get
        unsigned GetAudioInCount() override;
        unsigned GetAudioOutCount() override;
        bool Init() override;
        void PopulateOrExchange(QWidget *amplifyForm) override;
    private:
       double mPeak;
       double mRatio;
       double mRatioClip;
    };
}

#endif // AMPLIFY_H
