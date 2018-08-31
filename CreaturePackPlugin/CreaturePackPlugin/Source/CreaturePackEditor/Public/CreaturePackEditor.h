#pragma once

#include "Modules/ModuleManager.h"

class CreaturePackEditor : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	void StartupModule();
	void ShutdownModule();
};