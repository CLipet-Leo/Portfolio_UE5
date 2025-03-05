// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactible.h"
#include "DoorCpp.generated.h"

class UTimelineComponent;
class UCurveFloat;

UCLASS()
class PORTFOLIO_UE5_API ADoorCpp : public AActor, public IInteractible
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoorCpp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Implement the IECInteractable interface
	virtual void OnInteractBy_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable, Category = "Door")
	void OpenDoor();

	UFUNCTION(BlueprintCallable, Category = "Door")
	void CloseDoor();

private:
	UFUNCTION()
	void HandleDoorProgress(float Value);

	UFUNCTION()
	void InvertDoOnce();

private:
	UPROPERTY(VisibleAnywhere, Category = "Door", meta = (AllowPrivate))
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(VisibleAnywhere, Category = "Door", meta = (AllowPrivate))
	USceneComponent* Pivot;

	UPROPERTY(EditAnywhere, Category = "Door")
	UCurveFloat* DoorCurve;

	UPROPERTY()
	UTimelineComponent* DoorTimeline;

	// Variables for DoOnce and FlipFlop
	bool bHasOpenedOnce;
	bool bIsDoorOpen;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door", meta = (AllowPrivate))
	float TimeToOpen = 1.0f;
};
