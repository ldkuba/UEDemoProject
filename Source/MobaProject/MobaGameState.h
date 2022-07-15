// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "MobaGameState.generated.h"

USTRUCT(BlueprintType)
struct FScore
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Player1Score;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Player2Score;
};

/**
 * 
 */
UCLASS()
class MOBAPROJECT_API AMobaGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	static const int MAX_SCORE = 11;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetScore(const FScore& NewScore);
	inline FScore GetScore() const { return GameScore; }

protected:
	// Score
	UPROPERTY(ReplicatedUsing = OnRep_Score, VisibleAnywhere, BlueprintReadOnly)
	FScore GameScore;
	UFUNCTION(BlueprintImplementableEvent)
	void OnRep_Score();

};
