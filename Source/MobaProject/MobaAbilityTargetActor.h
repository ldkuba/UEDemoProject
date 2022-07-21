// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "MobaAbilityTargetActor.generated.h"

class AMobaController;

/**
 * 
 */
UCLASS()
class MOBAPROJECT_API AMobaAbilityTargetActor : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

	void BeginPlay() override;

public:
	AMobaAbilityTargetActor(const FObjectInitializer& ObjectInitializer);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void StartTargeting(UGameplayAbility* Ability) override;

protected:
	UPROPERTY(Replicated)
	AMobaController* OwningPlayerController;

	UFUNCTION(Server, Reliable)
	void ServerSendTargetingInput(FGameplayAbilityTargetDataHandle Location);

	FGameplayAbilityTargetDataHandle CalculateTargetData();

};
