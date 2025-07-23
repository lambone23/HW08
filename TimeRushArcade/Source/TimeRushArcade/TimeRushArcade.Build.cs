// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TimeRushArcade : ModuleRules
{
	public TimeRushArcade(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {  });
		
PublicIncludePaths.AddRange(
    new string[] {
        "TimeRushArcade/Public/Characters",
        "TimeRushArcade/Public/Items",
        "TimeRushArcade/Public/UI",
        "TimeRushArcade/Public/GameModes",
        "TimeRushArcade/Public/Controllers",
        "TimeRushArcade/Public/Core"
    }
);

PrivateIncludePaths.AddRange(
    new string[] {
        "TimeRushArcade/Private/Characters",
        "TimeRushArcade/Private/Items",
        "TimeRushArcade/Private/UI",
        "TimeRushArcade/Private/GameModes",
        "TimeRushArcade/Private/Controllers",
        "TimeRushArcade/Private/Core"
    }
);


		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
