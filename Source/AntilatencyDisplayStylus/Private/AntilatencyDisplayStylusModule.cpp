#include "IAntilatencyDisplayStylusModule.h"
#include "Modules/ModuleManager.h"
#include <Interfaces/IPluginManager.h>
#include "Misc/Paths.h"

class FAntilatencyDisplayStylusModule : public IAntilatencyDisplayStylusModule {
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override { return true; }
};

IMPLEMENT_MODULE(FAntilatencyDisplayStylusModule, AntilatencyDisplayStylus)


void FAntilatencyDisplayStylusModule::StartupModule() {
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}


void FAntilatencyDisplayStylusModule::ShutdownModule() {
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}