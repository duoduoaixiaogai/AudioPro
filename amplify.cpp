#include "amplify.h"

namespace RF {
    EffectAmplify::EffectAmplify() {

    }

    EffectAmplify::~EffectAmplify() {

    }

    ComponentInterfaceSymbol EffectAmplify::getSymbol()
    {
       return AMPLIFY_PLUGIN_SYMBOL;
    }

    unsigned EffectAmplify::GetAudioInCount()
    {
       return 1;
    }

    unsigned EffectAmplify::GetAudioOutCount()
    {
       return 1;
    }
}
