// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LevelEditor : ModuleRules
{
	public LevelEditor(TargetInfo Target)
	{
		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"AssetTools",
				"ClassViewer",
				"MainFrame",
                "PlacementMode",
				"ReferenceViewer",
				"SizeMap",
				"SlateReflector",
                "IntroTutorials",
                "AppFramework"
			}
		);

		PublicIncludePathModuleNames.AddRange(
			new string[] {
				"Settings",
				"UserFeedback",
				"IntroTutorials",
                "HierarchicalLODOutliner"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Analytics",
				"Core",
				"CoreUObject",
				"DesktopPlatform",
                "InputCore",
				"Slate",
				"SlateCore",
                "EditorStyle",
				"Engine",
				"MessageLog",
				"NewsFeed",
                "SourceControl",
                "SourceControlWindows",
                "StatsViewer",
				"UnrealEd", 
				"RenderCore",
				"DeviceProfileServices",
				"ContentBrowser",
                "SceneOutliner",
                "ActorPickerMode",
                "RHI",
				"Projects",
				"TargetPlatform",
				"EngineSettings",
				"PropertyEditor",
				"WebBrowser",
                "Persona",
                "Kismet",
				"KismetWidgets",
				"Sequencer",
                "Foliage"
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				"MainFrame",
				"ClassViewer",
				"DeviceManager",
				"SettingsEditor",
				"SessionFrontend",
				"SlateReflector",
				"AutomationWindow",
				"Layers",
                "WorldBrowser",
				"EditorWidgets",
				"AssetTools",
				"WorkspaceMenuStructure",
				"NewLevelDialog",
				"DeviceProfileEditor",
                "PlacementMode",
				"UserFeedback",
				"ReferenceViewer",
				"SizeMap",
                "IntroTutorials",
                "HierarchicalLODOutliner"
			}
		);
	}
}
