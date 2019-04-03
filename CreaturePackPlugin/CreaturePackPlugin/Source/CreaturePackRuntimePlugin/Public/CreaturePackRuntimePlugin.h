#pragma once

#include "Modules/ModuleManager.h"

class CreaturePackRuntimePlugin : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    void StartupModule();
    void ShutdownModule();
};