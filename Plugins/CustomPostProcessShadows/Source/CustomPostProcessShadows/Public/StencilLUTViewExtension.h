// StencilLUTViewExtension.h

#pragma once

#include "SceneViewExtension.h"
#include "RenderResource.h"
#include "RenderGraphResources.h"
#include "StencilMatrixLUTSubsystem.h"

// Forward declaration is enough for a pointer/reference
class UStencilMatrixLUTSubsystem;
DECLARE_LOG_CATEGORY_EXTERN(LogStencilLUT, Log, All);

class FStencilLUTViewExtension : public FSceneViewExtensionBase
{
public:
	FStencilLUTViewExtension(const FAutoRegister& AutoReg, UStencilMatrixLUTSubsystem* InWorldSubsystem);

	//~ Begin FSceneViewExtensionBase Interface
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {};
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override;
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;
	//~ End FSceneViewExtensionBase Interface

private:
	UStencilMatrixLUTSubsystem* WorldSubsystem;
};
