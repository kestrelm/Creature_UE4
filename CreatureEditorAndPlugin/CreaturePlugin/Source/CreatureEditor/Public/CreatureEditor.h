#pragma once

#include "Modules/ModuleManager.h"

class CreatureEditor : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	void StartupModule();
	void ShutdownModule();
};