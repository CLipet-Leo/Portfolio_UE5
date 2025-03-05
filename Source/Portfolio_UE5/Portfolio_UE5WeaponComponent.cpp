// Copyright Epic Games, Inc. All Rights Reserved.


#include "Portfolio_UE5WeaponComponent.h"
#include "Portfolio_UE5Character.h"
#include "Portfolio_UE5Projectile.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "NiagaraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UPortfolio_UE5WeaponComponent::UPortfolio_UE5WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
	
	// Create a spline component
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SplineComponent->SetupAttachment(this, TEXT("Muzzle"));
	SplineComponent->SetHiddenInGame(false, true);

	// Create a niagara component
	ParticleSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ParticleSystem"));
	ParticleSystem->SetupAttachment(SplineComponent);
	ParticleSystem->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
}


void UPortfolio_UE5WeaponComponent::Fire()
{
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}

	// Try and fire a projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);
	
			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	
			// Spawn the projectile at the muzzle
			World->SpawnActor<APortfolio_UE5Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}
	
	// Try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}
	
	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void UPortfolio_UE5WeaponComponent::PredictProjectilePath()
{
	// Destroy the previous spline meshes
	if (SplineMeshes.Num() > 0)
	{
		for (auto MeshToDestroy : SplineMeshes)
			MeshToDestroy->DestroyComponent();
		SplineMeshes.Empty();
	}

	// Calculate the vectors for prediction
	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
	const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);

	FPredictProjectilePathParams PredictProjectilePathParams;
	PredictProjectilePathParams.StartLocation = SpawnLocation;
	PredictProjectilePathParams.LaunchVelocity = SpawnRotation.Vector() * ProjectileClass.GetDefaultObject()->GetProjectileMovement()->InitialSpeed;
	PredictProjectilePathParams.ProjectileRadius = 5.f;
	PredictProjectilePathParams.ActorsToIgnore.Add(GetOwner());
	PredictProjectilePathParams.ActorsToIgnore.Add(PlayerController->GetPawn());
	PredictProjectilePathParams.bTraceWithCollision = true;

	FPredictProjectilePathResult PredictProjectilePathResult;
	bool bHit = UGameplayStatics::PredictProjectilePath(GetWorld(), PredictProjectilePathParams, PredictProjectilePathResult);

	TArray<FVector> PathPoints;
	for (auto PointData : PredictProjectilePathResult.PathData)
		PathPoints.Add(PointData.Location);

	SplineComponent->SetSplinePoints(PathPoints, ESplineCoordinateSpace::World);
	
	// Draw the predicted path
	for (int32 i = 0; i < PathPoints.Num() - 1; ++i)
	{
		/*FVector StartPos = PathPoints[i];
		FVector EndPos = PathPoints[i + 1];
		FVector StartTangent = SplineComponent->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World);
		FVector EndTangent = SplineComponent->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::World);

		USplineMeshComponent* SplineTrace = NewObject<USplineMeshComponent>(this);
		SplineTrace->SetMobility(EComponentMobility::Movable);
		SplineTrace->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepWorldTransform);
		SplineTrace->SetStaticMesh(SplineMesh);
		SplineTrace->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
		SplineTrace->RegisterComponent();
		SplineMeshes.Add(SplineTrace);*/

		//DrawDebugLine(GetWorld(), PathPoints[i], PathPoints[i + 1], FColor::Green, false, 0.1f, 0, 1.0f);
	}
}

bool UPortfolio_UE5WeaponComponent::AttachWeapon(APortfolio_UE5Character* TargetCharacter)
{
	Character = TargetCharacter;

	// Check that the character is valid, and has no weapon component yet
	if (Character == nullptr || Character->GetInstanceComponents().FindItemByClass<UPortfolio_UE5WeaponComponent>())
	{
		return false;
	}

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// Fire
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UPortfolio_UE5WeaponComponent::Fire);
		}
	}

	return true;
}

void UPortfolio_UE5WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// ensure we have a character owner
	if (Character != nullptr)
	{
		// remove the input mapping context from the Player Controller
		if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->RemoveMappingContext(FireMappingContext);
			}
		}
	}

	// maintain the EndPlay call chain
	Super::EndPlay(EndPlayReason);
}