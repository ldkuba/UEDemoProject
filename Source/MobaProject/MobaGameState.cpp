// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaGameState.h"

void AMobaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMobaGameState, GameScore);
}

void AMobaGameState::SetScore(const FScore& NewScore)
{
    GameScore = NewScore;
    OnRep_Score();
}
