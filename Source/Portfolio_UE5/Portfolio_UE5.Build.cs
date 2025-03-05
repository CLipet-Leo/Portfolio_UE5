// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Portfolio_UE5 : ModuleRules
{
	public Portfolio_UE5(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Niagara" });
	}
}
