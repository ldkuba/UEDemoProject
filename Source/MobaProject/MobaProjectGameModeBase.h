// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MobaUnit.h"
#include "MobaProjectGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class MOBAPROJECT_API AMobaProjectGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	AMobaProjectGameModeBase();

	void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
	TSubclassOf<AMobaUnit> DefaultCharacter;

protected:
	void OnPostLogin(AController* NewPlayer) override;
};
