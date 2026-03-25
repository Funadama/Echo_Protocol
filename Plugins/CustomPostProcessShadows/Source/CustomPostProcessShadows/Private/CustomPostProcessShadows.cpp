// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomPostProcessShadows.h"
#include "Interfaces/IPluginManager.h"
#include "StencilLUTViewExtension.h"

#define LOCTEXT_NAMESPACE "FCustomPostProcessShadowsModule"

void FCustomPostProcessShadowsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	//Uncomment this section if you would like to include additional shader files
	/*const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("CustomPostProcessShadows"));
	if (Plugin.IsValid()) {
		//The physical path on the disk for our .ush files
		const FString ShaderDir = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Shaders"));
		//The virtual path we will use when including this within the engine
		AddShaderSourceDirectoryMapping(TEXT("/Plugin/CustomPostProcessShadows"), ShaderDir);
	}*/
}

void FCustomPostProcessShadowsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCustomPostProcessShadowsModule, CustomPostProcessShadows)