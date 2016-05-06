#pragma once

#include "ModuleManager.h"

class CreaturePackEditor : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	void StartupModule();
	void ShutdownModule();
};