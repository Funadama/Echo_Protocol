// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "StencilLUTViewExtension.h"
#include "StencilMatrixLUTSubsystem.generated.h"

/**
 *
 */
UCLASS()
class CUSTOMPOSTPROCESSSHADOWS_API UStencilMatrixLUTSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//UTickableWorldSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UStencilMatrixLUTSubsystem, STATGROUP_Tickables);
	}
	FRHITexture* GetLUTTextureRHI_RenderThread();
	// public API
	//Texture to assign to our post-process material
	UFUNCTION(BlueprintCallable, Category = "StencilMatrixLUT")
	UTexture2D* GetMatrixLUTTexture() const { return MatrixLUTTexture; }
	UFUNCTION(BlueprintCallable, Category = "StencilMatrixLUT")
	void RegisterPrimitiveCandidate(UPrimitiveComponent* Primitive);
	UFUNCTION(BlueprintCallable, Category = "StencilMatrixLUT")
	void UnregisterPrimitiveCandidate(UPrimitiveComponent* Primitive);

private:
	static constexpr int32 LUT_Width = 256; // IDs [0..255]
	static constexpr int32 LUT_Height = 3;   // 3 rows per matrix

	// Keep track of used stencil IDs -> stack
	TArray<uint8> IdSlots;

	// The GPU texture used in material
	UPROPERTY()
	UTexture2D* MatrixLUTTexture = nullptr;

	bool bTextureInitialized = false;
	TSharedPtr<class FStencilLUTViewExtension, ESPMode::ThreadSafe> ViewExtension;
	//TArray<UPrimitiveComponent*> TrackedPrimitives;
	TArray<UPrimitiveComponent*> CandidatePrimitives;

private:
	void EnsureTextureCreated();
	void TrackRelevantPrimitives();
	bool AllocateId(uint8& OutId);
	void FreeStencilIdSlot(uint8 Slot);
	void AssignIdToPrimitive(UPrimitiveComponent* Primitive);
};
