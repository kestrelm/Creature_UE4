#pragma once

#include "ModuleManager.h"

class CreaturePlugin : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    void StartupModule();
    void ShutdownModule();
};