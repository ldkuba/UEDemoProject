// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MobaPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MOBAPROJECT_API AMobaPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	void OnRep_PlayerName() override;
	
};
