#pragma once

#include "ModuleManager.h"

class CreaturePackPlugin : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    void StartupModule();
    void ShutdownModule();
};