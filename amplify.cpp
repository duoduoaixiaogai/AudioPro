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
}
