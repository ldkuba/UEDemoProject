// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MobaController.h"
#include "MobaProjectGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class MOBAPROJECT_API AMobaProjectGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	const int32 MAX_PLAYERS = 2;

public:

	AMobaProjectGameModeBase();

	void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
	TSubclassOf<AMobaUnit> DefaultCharacter;

protected:
	void PostLogin(APlayerController* NewPlayer) override;

	void SpawnPlayerUnit(AMobaController* PlayerController);

	void RestartGame();
	void StartNewRound();

};
