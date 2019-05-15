#ifndef AMPLIFY_H
#define AMPLIFY_H

#include "effect.h"
#include "componentinterface.h"
#include "effectinterface.h"

namespace Renfeng {
#define AMPLIFY_PLUGIN_SYMBOL ComponentInterfaceSymbol(QString("Amplify"))

  class AmplifyForm;

    class EffectAmplify final : public Effect {
      Q_OBJECT
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
        void PopulateOrExchange(QWidget *parent) override;
    private:
        void CheckClip(QWidget *amplifyForm);
    private slots:
        void OnAmpSlider(int value);
        void OnAmpText(const QString &value);
        void OnPeakText(const QString &value);
        void apply();
    private:
       double mPeak;
       double mRatio;
       double mRatioClip;
       double mAmp;
       double mNewPeak;
       bool mCanClip;

        AmplifyForm *mForm;
    };
}

#endif // AMPLIFY_H
