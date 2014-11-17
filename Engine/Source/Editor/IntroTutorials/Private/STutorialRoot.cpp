// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "IntroTutorialsPrivatePCH.h"
#include "STutorialRoot.h"
#include "SEditorTutorials.h"
#include "EditorTutorialSettings.h"
#include "TutorialStateSettings.h"
#include "AssetEditorManager.h"
#include "ToolkitManager.h"
#include "IToolkit.h"
#include "IToolkitHost.h"
#include "EngineAnalytics.h"
#include "Runtime/Analytics/Analytics/Public/Interfaces/IAnalyticsProvider.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"

#define LOCTEXT_NAMESPACE "STutorialRoot"

void STutorialRoot::Construct(const FArguments& InArgs)
{
	CurrentTutorial = nullptr;
	CurrentTutorialStage = 0;
	CurrentTutorialStartTime = FPlatformTime::Seconds();

	ChildSlot
	[
		SNullWidget::NullWidget
	];
}

void STutorialRoot::MaybeAddOverlay(TSharedRef<SWindow> InWindow)
{
	if(InWindow->HasOverlay())
	{
		// check we dont already have a widget overlay for this window
		TWeakPtr<SEditorTutorials>* FoundWidget = TutorialWidgets.Find(InWindow);
		if(FoundWidget == nullptr)
		{
			TSharedPtr<SEditorTutorials> TutorialWidget = nullptr;
			InWindow->AddOverlaySlot()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Fill)
				[
					SAssignNew(TutorialWidget, SEditorTutorials)
					.ParentWindow(InWindow)
					.OnNextClicked(FOnNextClicked::CreateSP(this, &STutorialRoot::HandleNextClicked))
					.OnBackClicked(FSimpleDelegate::CreateSP(this, &STutorialRoot::HandleBackClicked))
					.OnHomeClicked(FSimpleDelegate::CreateSP(this, &STutorialRoot::HandleHomeClicked))
					.OnCloseClicked(FSimpleDelegate::CreateSP(this, &STutorialRoot::HandleCloseClicked))
					.OnGetCurrentTutorial(FOnGetCurrentTutorial::CreateSP(this, &STutorialRoot::HandleGetCurrentTutorial))
					.OnGetCurrentTutorialStage(FOnGetCurrentTutorialStage::CreateSP(this, &STutorialRoot::HandleGetCurrentTutorialStage))
					.OnLaunchTutorial(FOnLaunchTutorial::CreateSP(this, &STutorialRoot::LaunchTutorial))
					.OnWasWidgetDrawn(FOnWasWidgetDrawn::CreateSP(this, &STutorialRoot::WasWidgetDrawn))
					.OnWidgetWasDrawn(FOnWidgetWasDrawn::CreateSP(this, &STutorialRoot::WidgetWasDrawn))
				]
			];

			FoundWidget = &TutorialWidgets.Add(InWindow, TutorialWidget);

			FoundWidget->Pin()->RebuildCurrentContent();
		}
	}

	TArray<TSharedRef<SWindow>> ChildWindows = InWindow->GetChildWindows();
	for(auto& ChildWindow : ChildWindows)
	{
		MaybeAddOverlay(ChildWindow);
	}
}

void STutorialRoot::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	TArray<TSharedRef<SWindow>> Windows = FSlateApplication::Get().GetInteractiveTopLevelWindows();
	for(auto& Window : Windows)
	{
		MaybeAddOverlay(Window);
	}

	// empty array but leave us the slack (we dont want to reallocate all the time, and this array should never grow too large)
	PreviouslyDrawnWidgets.Empty(PreviouslyDrawnWidgets.Max());
	PreviouslyDrawnWidgets.Append(MoveTemp(DrawnWidgets));
	DrawnWidgets.Empty(DrawnWidgets.Max());
}

void STutorialRoot::SummonTutorialBrowser(TWeakPtr<SWindow> InWindow, const FString& InFilter)
{
	// Use main frame if non specified
	if (!InWindow.IsValid())
	{
		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		InWindow = MainFrameModule.GetParentWindow();
	}

	TWeakPtr<SEditorTutorials>* FoundWidget = TutorialWidgets.Find(InWindow);
	if(FoundWidget != nullptr && FoundWidget->IsValid())
	{
		FoundWidget->Pin()->ShowBrowser(InFilter);
	}
}

void STutorialRoot::LaunchTutorial(UEditorTutorial* InTutorial, bool bInRestart, TWeakPtr<SWindow> InNavigationWindow, FSimpleDelegate InOnTutorialClosed, FSimpleDelegate InOnTutorialExited)
{
	if(InTutorial != nullptr)
	{
		CurrentTutorial = InTutorial;

		// we force a restart if this tutorial was completed
		if(GetDefault<UTutorialStateSettings>()->HaveCompletedTutorial(CurrentTutorial))
		{
			bInRestart = true;
		}

		bool bHaveSeenTutorial = false;
		CurrentTutorialStage = bInRestart ? 0 : GetDefault<UTutorialStateSettings>()->GetProgress(CurrentTutorial, bHaveSeenTutorial);

		// check if we should be launching this tutorial for an asset editor
		if(InTutorial->AssetToUse.IsValid())
		{
			TArray<FString> AssetPaths;
			AssetPaths.Add(InTutorial->AssetToUse.AssetLongPathname);
			FAssetEditorManager::Get().OpenEditorsForAssets(AssetPaths);

			UObject* Asset = InTutorial->AssetToUse.ResolveObject();
			if(Asset != nullptr)
			{
				TSharedPtr<IToolkit> Toolkit = FToolkitManager::Get().FindEditorForAsset( Asset );
				if(Toolkit.IsValid())
				{
					InNavigationWindow = FSlateApplication::Get().FindWidgetWindow(Toolkit->GetToolkitHost()->GetParentWidget());

					// make sure we have a valid tutorial overlay
					if(InNavigationWindow.IsValid())
					{
						MaybeAddOverlay(InNavigationWindow.Pin().ToSharedRef());
					}
				}
			}
		}

		CurrentTutorialStartTime = FPlatformTime::Seconds();

		// launch tutorial for all windows we wrap - any tutorial can display over any window
		for(auto& TutorialWidget : TutorialWidgets)
		{
			if(TutorialWidget.Value.IsValid())
			{
				bool bIsNavigationWindow = false;
				if (!InNavigationWindow.IsValid())
				{
					bIsNavigationWindow = TutorialWidget.Value.Pin()->IsNavigationVisible();
				}
				else
				{
					bIsNavigationWindow = (TutorialWidget.Value.Pin()->GetParentWindow() == InNavigationWindow.Pin());
				}
				TutorialWidget.Value.Pin()->LaunchTutorial(bIsNavigationWindow, InOnTutorialClosed, InOnTutorialExited);
			}
		}
	}
}

void STutorialRoot::CloseAllTutorialContent()
{
	for (auto& TutorialWidget : TutorialWidgets)
	{
		if (TutorialWidget.Value.IsValid())
		{
			TutorialWidget.Value.Pin()->HideContent();
		}
	}
}

void STutorialRoot::HandleNextClicked(TWeakPtr<SWindow> InNavigationWindow)
{
	GoToNextStage(InNavigationWindow);

	for(auto& TutorialWidget : TutorialWidgets)
	{
		if(TutorialWidget.Value.IsValid())
		{
			TSharedPtr<SEditorTutorials> PinnedTutorialWidget = TutorialWidget.Value.Pin();
			PinnedTutorialWidget->RebuildCurrentContent();
		}
	}
}

void STutorialRoot::HandleBackClicked()
{
	if( FEngineAnalytics::IsAvailable() && CurrentTutorial != nullptr)
	{
		TArray<FAnalyticsEventAttribute> EventAttributes;
		EventAttributes.Add(FAnalyticsEventAttribute(TEXT("Context.Tutorial"), FIntroTutorials::AnalyticsEventNameFromTutorial(TEXT(""), CurrentTutorial)));
		EventAttributes.Add(FAnalyticsEventAttribute(TEXT("Context.StageIndex"), CurrentTutorialStage));

		FEngineAnalytics::GetProvider().RecordEvent( TEXT("Rocket.Tutorials.ClickedBackButton"), EventAttributes );
	}

	GoToPreviousStage();

	for(auto& TutorialWidget : TutorialWidgets)
	{
		if(TutorialWidget.Value.IsValid())
		{
			TSharedPtr<SEditorTutorials> PinnedTutorialWidget = TutorialWidget.Value.Pin();
			PinnedTutorialWidget->RebuildCurrentContent();
		}
	}
}

void STutorialRoot::HandleHomeClicked()
{
	if(CurrentTutorial != nullptr)
	{
		GetMutableDefault<UTutorialStateSettings>()->RecordProgress(CurrentTutorial, CurrentTutorialStage);
		GetMutableDefault<UTutorialStateSettings>()->SaveProgress();
	}

	CurrentTutorial = nullptr;
	CurrentTutorialStage = 0;

	for(auto& TutorialWidget : TutorialWidgets)
	{
		if(TutorialWidget.Value.IsValid())
		{
			TSharedPtr<SEditorTutorials> PinnedTutorialWidget = TutorialWidget.Value.Pin();
			PinnedTutorialWidget->RebuildCurrentContent();
		}
	}
}

UEditorTutorial* STutorialRoot::HandleGetCurrentTutorial()
{
	return CurrentTutorial;
}

int32 STutorialRoot::HandleGetCurrentTutorialStage()
{
	return CurrentTutorialStage;
}

void STutorialRoot::AddReferencedObjects( FReferenceCollector& Collector )
{
	if(CurrentTutorial != nullptr)
	{
		Collector.AddReferencedObject(CurrentTutorial);
	}
}

void STutorialRoot::GoToPreviousStage()
{
	if(CurrentTutorial != nullptr)
	{
		int32 PreviousTutorialStage = CurrentTutorialStage;
		if (CurrentTutorialStage > 0)
		{
			CurrentTutorial->HandleTutorialStageEnded(CurrentTutorial->Stages[CurrentTutorialStage].Name);
		}

		CurrentTutorialStage = FMath::Max(0, CurrentTutorialStage - 1);

		if (PreviousTutorialStage != CurrentTutorialStage)
		{
			CurrentTutorial->HandleTutorialStageStarted(CurrentTutorial->Stages[CurrentTutorialStage].Name);
		}
	}
}

void STutorialRoot::GoToNextStage(TWeakPtr<SWindow> InNavigationWindow)
{
	if(CurrentTutorial != nullptr)
	{
		UEditorTutorial* PreviousTutorial = CurrentTutorial;
		int32 PreviousTutorialStage = CurrentTutorialStage;
		if(CurrentTutorialStage < CurrentTutorial->Stages.Num())
		{
			CurrentTutorial->HandleTutorialStageEnded(CurrentTutorial->Stages[CurrentTutorialStage].Name);
		}

		if(CurrentTutorialStage + 1 >= CurrentTutorial->Stages.Num() && FName(*CurrentTutorial->NextTutorial.AssetLongPathname) != NAME_None)
		{
			TSubclassOf<UEditorTutorial> NextTutorialClass = LoadClass<UEditorTutorial>(NULL, *CurrentTutorial->NextTutorial.AssetLongPathname, NULL, LOAD_None, NULL);
			if(NextTutorialClass != nullptr)
			{
				LaunchTutorial(NextTutorialClass->GetDefaultObject<UEditorTutorial>(), true, InNavigationWindow, FSimpleDelegate(), FSimpleDelegate());
			}
			else
			{
				FSlateNotificationManager::Get().AddNotification(FNotificationInfo(FText::Format(LOCTEXT("TutorialNotFound", "Could not start next tutorial {0}"), FText::FromString(CurrentTutorial->NextTutorial.AssetLongPathname))));
			}
		}
		else
		{
			CurrentTutorialStage = FMath::Min(CurrentTutorialStage + 1, CurrentTutorial->Stages.Num() - 1);
			GetMutableDefault<UTutorialStateSettings>()->RecordProgress(CurrentTutorial, CurrentTutorialStage);
		}

		if (CurrentTutorial != nullptr && CurrentTutorialStage < CurrentTutorial->Stages.Num() && (CurrentTutorial != PreviousTutorial || CurrentTutorialStage != PreviousTutorialStage))
		{
			CurrentTutorial->HandleTutorialStageStarted(CurrentTutorial->Stages[CurrentTutorialStage].Name);
		}
	}
}

void STutorialRoot::HandleCloseClicked()
{
	// submit analytics data
	if( FEngineAnalytics::IsAvailable() && CurrentTutorial != nullptr && CurrentTutorialStage < CurrentTutorial->Stages.Num() )
	{
		UEditorTutorial* AttractTutorial = nullptr;
		UEditorTutorial* LaunchTutorial = nullptr;
		FString BrowserFilter;
		GetDefault<UEditorTutorialSettings>()->FindTutorialInfoForContext(TEXT("LevelEditor"), AttractTutorial, LaunchTutorial, BrowserFilter);

		// prepare and send analytics data
		bool const bClosedInitialAttract = (CurrentTutorial == AttractTutorial);

		FString const CurrentExcerptTitle = bClosedInitialAttract ? TEXT("InitialAttract") : CurrentTutorial->Stages[CurrentTutorialStage].Name.ToString();
		int32 const CurrentExcerptIndex = bClosedInitialAttract ? -1 : CurrentTutorialStage;
		float const CurrentPageElapsedTime = bClosedInitialAttract ? 0.f : (float)(FPlatformTime::Seconds() - CurrentTutorialStartTime);

		TArray<FAnalyticsEventAttribute> EventAttributes;
		EventAttributes.Add(FAnalyticsEventAttribute(TEXT("LastStageIndex"), CurrentExcerptIndex));
		EventAttributes.Add(FAnalyticsEventAttribute(TEXT("LastStageTitle"), CurrentExcerptTitle));
		EventAttributes.Add(FAnalyticsEventAttribute(TEXT("TimeSpentInTutorial"), CurrentPageElapsedTime));
			
		FEngineAnalytics::GetProvider().RecordEvent( FIntroTutorials::AnalyticsEventNameFromTutorial(TEXT("Rocket.Tutorials.Closed"), CurrentTutorial), EventAttributes );
	}
}

bool STutorialRoot::WasWidgetDrawn(const FName& InName) const
{
	for(const auto& WidgetName : PreviouslyDrawnWidgets)
	{
		if(InName == WidgetName)
		{
			return true;
		}
	}

	return false;
}

void STutorialRoot::WidgetWasDrawn(const FName& InName)
{
	DrawnWidgets.Add(InName);
}

#undef LOCTEXT_NAMESPACE