// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AutoSurvivor : ModuleRules
{
    public AutoSurvivor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // I added "EnhancedInput" to this list so the error goes away
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
    }
}