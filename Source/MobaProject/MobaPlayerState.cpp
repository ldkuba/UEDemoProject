// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaPlayerState.h"
#include "MobaController.h"

void AMobaPlayerState::OnRep_PlayerName()
{
    Super::OnRep_PlayerName();

    if(GetNetMode() != ENetMode::NM_Client)
    {
        GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Purple, FString::Printf(TEXT("Player name changed on server: %s"), *GetPlayerName()));

        AMobaController* controller = Cast<AMobaController>(GetOwningController());
        if(controller)
        {
            controller->SetPlayerName(GetPlayerName());
        }
    }
}
