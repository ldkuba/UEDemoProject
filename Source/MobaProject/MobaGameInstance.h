// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MobaGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MOBAPROJECT_API UMobaGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;
	
};
