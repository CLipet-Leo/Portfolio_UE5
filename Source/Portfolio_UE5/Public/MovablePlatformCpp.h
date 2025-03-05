// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovablePlatformCpp.generated.h"

UCLASS()
class PORTFOLIO_UE5_API AMovablePlatformCpp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMovablePlatformCpp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to set the platform's target location
	UFUNCTION(BlueprintCallable, Category = "Platform")
	void SetTargetLocation(FVector TargetLocation);

	UFUNCTION(BlueprintCallable, Category = "Platform")
	FVector CalculateDistanceToTarget();

	// Called to move the platform to the target location
	UFUNCTION(BlueprintCallable, Category = "Platform")
	void MoveToTargetLocation(float DeltaTime);

	// Called to change the current actor
	UFUNCTION(BlueprintCallable, Category = "Platform")
	void ChangeSelectedActor();

private:
	UPROPERTY(EditAnywhere, Category = "Platform")
	TArray<AActor*> Actors;

	UPROPERTY()
	AActor* SelectedActor;

	UPROPERTY(VisibleAnywhere, Category = "Platform")
	UStaticMeshComponent* PlatformMesh;

	UPROPERTY(VisibleAnywhere, Category = "Platform")
	FVector CurrentTargetLocation;

	UPROPERTY(EditAnywhere, Category = "Platform")
	float Speed = 200.f;
};
