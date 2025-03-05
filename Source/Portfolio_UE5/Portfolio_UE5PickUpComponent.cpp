// Copyright Epic Games, Inc. All Rights Reserved.

#include "Portfolio_UE5PickUpComponent.h"

UPortfolio_UE5PickUpComponent::UPortfolio_UE5PickUpComponent()
{
	// Setup the Sphere Collision
	SphereRadius = 32.f;
}

void UPortfolio_UE5PickUpComponent::BeginPlay()
{
	Super::BeginPlay();

	// Register our Overlap Event
	OnComponentBeginOverlap.AddDynamic(this, &UPortfolio_UE5PickUpComponent::OnSphereBeginOverlap);
}

void UPortfolio_UE5PickUpComponent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Checking if it is a First Person Character overlapping
	APortfolio_UE5Character* Character = Cast<APortfolio_UE5Character>(OtherActor);
	if(Character != nullptr)
	{
		// Notify that the actor is being picked up
		OnPickUp.Broadcast(Character);

		// Unregister from the Overlap Event so it is no longer triggered
		OnComponentBeginOverlap.RemoveAll(this);
	}
}
