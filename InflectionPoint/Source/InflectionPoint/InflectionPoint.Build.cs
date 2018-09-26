// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class InflectionPoint : ModuleRules {
    //public InflectionPoint(TargetInfo Target) {
    public InflectionPoint(ReadOnlyTargetRules Target) : base(Target) {
        MinFilesUsingPrecompiledHeaderOverride = 1;
        bFasterWithoutUnity = true;
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "UMG", "OnlineSubsystem", "OnlineSubsystemUtils", "LevelSequence", "MovieScene", "SlateCore", "MoviePlayer" });
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
        DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
        //DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");
    }
}
