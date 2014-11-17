// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameplayAbility.h"
#include "GameplayAbilityTargetActor.generated.h"

/** TargetActors are spawned to assist with ability targeting. They are spawned by ability tasks and create/determine the outgoing targeting data passed from one task to another. */
UCLASS(Blueprintable, abstract, notplaceable)
class GAMEPLAYABILITIES_API AGameplayAbilityTargetActor : public AActor
{
	GENERATED_UCLASS_BODY()

public:

	/** Native classes can set this to call StaticGetTargetData to instantly get TargetData, instead of instantiating the GameplayTargetActor and calling StartTargeting. */
	UPROPERTY()
	bool StaticTargetFunction;

	/** The TargetData this class produces can be entirely generated on the server. We don't require the client to send us full or partial TargetData (possibly just a 'confirm') */
	UPROPERTY()
	bool ShouldProduceTargetDataOnServer;

	/** Describes where the targeting action starts, usually the player character or a socket on the player character. */
	//UPROPERTY(BlueprintReadOnly, meta=(ExposeOnSpawn=true), Category=Targeting)
	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn = true), Replicated, Category = Targeting)
	FGameplayAbilityTargetingLocationInfo StartLocation;

	virtual FGameplayAbilityTargetDataHandle StaticGetTargetData(UWorld * World, const FGameplayAbilityActorInfo* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo) const;
	
	/** Initialize and begin targeting logic  */
	virtual void StartTargeting(UGameplayAbility* Ability);

	/** Requesting targeting data, but not necessarily stopping/destroying the task. Useful for external target data requests. */
	virtual void ConfirmTargetingAndContinue();

	/** Outside code is saying 'stop and just give me what you have' */
	UFUNCTION()
	virtual void ConfirmTargeting();

	/** Outside code is saying 'stop everything and just forget about it' */
	UFUNCTION()
	virtual void CancelTargeting();

	virtual void BindToConfirmCancelInputs();

	virtual bool ShouldProduceTargetData() const;

	/** Replicated target data was received from a client. Possibly sanitize/verify. return true if data is good and we should broadcast it as valid data. */
	virtual bool OnReplicatedTargetDataReceived(FGameplayAbilityTargetDataHandle& Data) const;

	/** Accessor for checking, before instantiating, if this TargetActor will replicate. */
	bool GetReplicates() const;

	// ------------------------------
	
	FAbilityTargetData	TargetDataReadyDelegate;
	FAbilityTargetData	CanceledDelegate;

	virtual bool IsNetRelevantFor(class APlayerController* RealViewer, AActor* Viewer, const FVector& SrcLocation) override;

	UPROPERTY(BlueprintReadOnly, Category = "Targeting")
	APlayerController* MasterPC;

	UPROPERTY()
	UGameplayAbility* OwningAbility;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = Targeting)
	bool bDestroyOnConfirmation;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = Targeting)
	AActor* SourceActor;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = Targeting)
	bool bDebug;
};