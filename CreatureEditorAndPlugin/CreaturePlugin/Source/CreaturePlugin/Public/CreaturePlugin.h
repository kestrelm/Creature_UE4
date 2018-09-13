#pragma once

#include "Modules/ModuleManager.h"

class CreaturePlugin : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    void StartupModule();
    void ShutdownModule();
};