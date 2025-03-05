// Copyright Epic Games, Inc. All Rights Reserved.

#include "Portfolio_UE5Character.h"
#include "Portfolio_UE5Projectile.h"
#include "Portfolio_UE5WeaponComponent.h"
#include "Interactible.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// APortfolio_UE5Character

APortfolio_UE5Character::APortfolio_UE5Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

}

void APortfolio_UE5Character::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (UserWidgetClass)
	{
		UserWidgetClass->AddToViewport();
	}
}

void APortfolio_UE5Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if the player is looking to a physical object
	if (IsLookingToPhysicalObject())
		OnLookingToPhysicalObject.Broadcast(true);
	else
		OnLookingToPhysicalObject.Broadcast(false);

	// Show a prediction of the projectile path
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	for (AActor* AttachedActor : AttachedActors)
	{
		TSet<UActorComponent*> Components = AttachedActor->GetComponents();
		for (UActorComponent* Component : Components)
		{
			if (UPortfolio_UE5WeaponComponent* WeaponComponent = Cast<UPortfolio_UE5WeaponComponent>(Component))
			{
				WeaponComponent->PredictProjectilePath();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////// Input

void APortfolio_UE5Character::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APortfolio_UE5Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APortfolio_UE5Character::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APortfolio_UE5Character::Look);
		
		// Raycasting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APortfolio_UE5Character::Interact);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void APortfolio_UE5Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void APortfolio_UE5Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APortfolio_UE5Character::Interact()
{
	FVector StartPos = FirstPersonCameraComponent->GetComponentLocation();
	FVector EndPos = StartPos + FirstPersonCameraComponent->GetForwardVector() * 1000.f;
	if (Controller != nullptr && Controller->IsLocalPlayerController())
	{
		// Perform an interaction
		FHitResult Hit(ForceInit);
		if (PerformRaycast(Hit, StartPos, EndPos, true))
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor->Implements<UInteractible>()) // Check if the actor implements the IECInteractable interface
			{
				IInteractible::Execute_OnInteractBy(HitActor, this);
			}
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("No Controller found!"));
	}
}

bool APortfolio_UE5Character::IsLookingToPhysicalObject()
{
	FVector StartPos = FirstPersonCameraComponent->GetComponentLocation();
	FVector EndPos = StartPos + FirstPersonCameraComponent->GetForwardVector() * 2000.f;
	FHitResult Hit(ForceInit);
	if (PerformRaycast(Hit, StartPos, EndPos))
	{
		AActor* HitActor = Hit.GetActor();
		// Parcourir tous les composants primitifs de l'acteur
		TArray<UPrimitiveComponent*> PrimitiveComponents;
		HitActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		for (UPrimitiveComponent* Component : PrimitiveComponents)
		{
			if (Component && Component->IsSimulatingPhysics())
			{
				return true;
			}
		}
	}
	return false;
}

bool APortfolio_UE5Character::PerformRaycast(FHitResult& Hit, FVector& StartPos, FVector& EndPos, bool debugLine)
{
	// Perform a raycast from the start position to the end position
	FCollisionQueryParams CollisionParams(FName(TEXT("InteractTrace")), true, this);
	if (debugLine)
	{
		// Set up a trace tag for debugging
		const FName TraceTag("MyTraceTag");
		GetWorld()->DebugDrawTraceTag = TraceTag;
		CollisionParams.TraceTag = TraceTag;
		CollisionParams.bDebugQuery = true;
	}
	return GetWorld()->LineTraceSingleByChannel(Hit, StartPos, EndPos, ECC_Visibility, CollisionParams);
}
