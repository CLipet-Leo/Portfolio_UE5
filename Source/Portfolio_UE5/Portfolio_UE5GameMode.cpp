// Copyright Epic Games, Inc. All Rights Reserved.

#include "Portfolio_UE5GameMode.h"
#include "Portfolio_UE5Character.h"
#include "UObject/ConstructorHelpers.h"

APortfolio_UE5GameMode::APortfolio_UE5GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
