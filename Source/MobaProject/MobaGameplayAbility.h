// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MobaGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class MOBAPROJECT_API UMobaGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void ConfirmCast();
};
