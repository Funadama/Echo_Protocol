// Fill out your copyright notice in the Description page of Project Settings.


#include "StencilCandidateComponent.h"

// Sets default values for this component's properties
UStencilCandidateComponent::UStencilCandidateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UStencilCandidateComponent::SetPrimitiveComponent(UPrimitiveComponent* InPrimitiveComponent)
{
	PrimitiveComponent = InPrimitiveComponent;
	// If the world is valid, send the primitive to the subsystem to register
	UWorld* World = PrimitiveComponent->GetWorld();
	if (World) {
		UStencilMatrixLUTSubsystem* Subsystem = World->GetSubsystem<UStencilMatrixLUTSubsystem>();
		if (Subsystem) {
			Subsystem->RegisterPrimitiveCandidate(PrimitiveComponent);
		}
	}
}

// Called when the game starts
void UStencilCandidateComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UStencilCandidateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

