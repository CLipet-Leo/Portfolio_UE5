// Fill out your copyright notice in the Description page of Project Settings.


#include "MovablePlatformCpp.h"

// Sets default values
AMovablePlatformCpp::AMovablePlatformCpp()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create and attach the PlatformMesh component
	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(RootComponent);

	// Set the default static mesh to a plane
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		PlatformMesh->SetStaticMesh(CubeMesh.Object);
		PlatformMesh->SetRelativeScale3D(FVector(2, 2, 0.1));
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> CubeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
		if (CubeMaterial.Succeeded())
			PlatformMesh->SetMaterial(0, CubeMaterial.Object);
	}

	// Initialize the actors array
	Actors = TArray<AActor*>();

	// Initialize the selected actor
	SelectedActor = nullptr;
}

// Called when the game starts or when spawned
void AMovablePlatformCpp::BeginPlay()
{
	Super::BeginPlay();
	
	if (Actors.Num() > 0)
	{
		SelectedActor = Actors[0];
		SetTargetLocation(SelectedActor->GetActorLocation());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No actors have been added to the platform."));
	}
}

// Called every frame
void AMovablePlatformCpp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Move the platform to the target location
	MoveToTargetLocation(DeltaTime);

	if (GetActorLocation() == CurrentTargetLocation)
	{
		ChangeSelectedActor();
		SetTargetLocation(SelectedActor->GetActorLocation());
	}
}

void AMovablePlatformCpp::SetTargetLocation(FVector TargetLocation)
{
	// Set the target location
	CurrentTargetLocation = TargetLocation;
}

FVector AMovablePlatformCpp::CalculateDistanceToTarget()
{
	// Calculate the distance to the target location
	return CurrentTargetLocation - GetActorLocation();
}

void AMovablePlatformCpp::MoveToTargetLocation(float DeltaTime)
{
	// Calculate the distance to the target location
	FVector Distance = CalculateDistanceToTarget();
	// Normalize the distance
	Distance.Normalize();
	// Calculate the new location
	FVector NewLocation = GetActorLocation() + Distance * Speed * DeltaTime;
	// Move the platform to the target location
	if (FVector::Dist(NewLocation, CurrentTargetLocation) > 5.0f)
		SetActorLocation(GetActorLocation() + Distance * Speed * DeltaTime);
	else
		SetActorLocation(CurrentTargetLocation);
}

void AMovablePlatformCpp::ChangeSelectedActor()
{
	for (int i = 0; i < Actors.Num(); i++)
	{
		if (Actors[i] == SelectedActor)
		{
			if (i == Actors.Num() - 1)
			{
				SelectedActor = Actors[0];
				break;
			}
			SelectedActor = Actors[i + 1];
			break;
		}
	}
}

