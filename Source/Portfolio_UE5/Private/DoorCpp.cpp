// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorCpp.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveFloat.h"

// Sets default values
ADoorCpp::ADoorCpp()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create and attach a Pivot component
	Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	Pivot->SetupAttachment(RootComponent);

	// Create and attach the DoorMesh component
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(Pivot);

	// Set the default static mesh to a cube
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		DoorMesh->SetStaticMesh(CubeMesh.Object);
		DoorMesh->SetRelativeScale3D(FVector(4, 0.3, 2));
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> CubeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
		if (CubeMaterial.Succeeded())
			DoorMesh->SetMaterial(0, CubeMaterial.Object);

		FVector DoorSize = DoorMesh->Bounds.BoxExtent * DoorMesh->GetRelativeScale3D();
		// Set the pivot location to the bottom left of the cube
		Pivot->SetRelativeLocation(FVector(-DoorSize.X, DoorSize.Y, 0));
		DoorMesh->SetRelativeLocation(FVector(DoorSize.X, -DoorSize.Y, 0));
	}

	// Initialize the timeline component
	DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));

	// Initialize the curve float
	static ConstructorHelpers::FObjectFinder<UCurveFloat> Curve(TEXT("/Game/Other/LinearCurveFloat"));
	if (Curve.Succeeded())
	{
		DoorCurve = Curve.Object;
	}

	// Initialize DoOnce and FlipFlop variables
	bHasOpenedOnce = false;
	bIsDoorOpen = false;
}

// Called when the game starts or when spawned
void ADoorCpp::BeginPlay()
{
	Super::BeginPlay();

	if (DoorCurve)
	{
		FOnTimelineFloat ProgressFunction;
		ProgressFunction.BindUFunction(this, FName("HandleDoorProgress"));

		DoorTimeline->AddInterpFloat(DoorCurve, ProgressFunction);
		DoorTimeline->SetLooping(false);
		DoorTimeline->SetIgnoreTimeDilation(true);
		DoorTimeline->SetTimelineLength(TimeToOpen);
		FOnTimelineEvent TimelineFinishedFunc;
		TimelineFinishedFunc.BindUFunction(this, FName("InvertDoOnce"));
		DoorTimeline->SetTimelineFinishedFunc(TimelineFinishedFunc);
	}
}

void ADoorCpp::OnInteractBy_Implementation(AActor* Interactor)
{
	// FlipFlop functionality
	if (bIsDoorOpen)
	{
		CloseDoor();
	}
	else
	{
		OpenDoor();
	}
}

void ADoorCpp::OpenDoor()
{
	// DoOnce functionality
	if (!bHasOpenedOnce)
	{
		if (DoorTimeline)
		{
			DoorTimeline->PlayFromStart();
		}
		InvertDoOnce();
		bIsDoorOpen = true;
	}
}

void ADoorCpp::CloseDoor()
{
	if (!bHasOpenedOnce)
	{
		if (DoorTimeline)
		{
			DoorTimeline->ReverseFromEnd();
		}
		InvertDoOnce();
		bIsDoorOpen = false;
	}
}

void ADoorCpp::HandleDoorProgress(float Value)
{
	FRotator NewRotation = FRotator(0.0f, Value * 90.0f, 0.0f);
	Pivot->SetRelativeRotation(NewRotation);
}

void ADoorCpp::InvertDoOnce()
{
	bHasOpenedOnce = !bHasOpenedOnce;
}
