// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StencilMatrixLUTSubsystem.h"
#include "StencilCandidateComponent.generated.h"

// Note that each stencil actor candidate can only have one mesh component writing to the stencil buffer
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CUSTOMPOSTPROCESSSHADOWS_API UStencilCandidateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStencilCandidateComponent();
	UFUNCTION(BlueprintCallable, Category = "StencilCandidate")
	void SetPrimitiveComponent(UPrimitiveComponent* InPrimitiveComponent);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	// The primitive component that will write to the stencil buffer
	UPrimitiveComponent* PrimitiveComponent;	
};
