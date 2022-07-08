// Copyright Epic Games, Inc. All Rights Reserved.

#include "MobaProjectGameModeBase.h"
#include "MobaPlayerState.h"

AMobaProjectGameModeBase::AMobaProjectGameModeBase()
    :Super()
{
    DefaultCharacter = TSubclassOf<AMobaUnit>(AMobaUnit::StaticClass());
}

void AMobaProjectGameModeBase::OnPostLogin(AController* NewPlayer)
{
    Super::OnPostLogin(NewPlayer);

    if(!NewPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("New player controller is null"));
        return;
    }

    AMobaPlayerState* playerState = NewPlayer->GetPlayerState<AMobaPlayerState>();
    if(playerState)
    {
        // Can only spawn pawns on server
        checkf(!IsNetMode(ENetMode::NM_Client), TEXT("SpawnPlayerCharacter() can only be called on the server"));

        APawn* pawn = NewPlayer->GetPawn();

        FActorSpawnParameters params;
        params.Owner = NewPlayer;

        AMobaUnit* playerUnit = GetWorld()->SpawnActor<AMobaUnit>(
            DefaultCharacter,
            pawn ? pawn->GetActorLocation() : FVector(200.0f, 200.0f, 100.0f),
            pawn ? pawn->GetActorRotation() : FRotator::ZeroRotator,
            params
        );

        // TODO: change to AddPlayerUnit() to handle multiple controllable units
        playerState->SetPlayerUnit(playerUnit);
    }

    if(GEngine)
    {
        if(GetNetMode() == ENetMode::NM_Client)
        {
            GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Green, FString::Printf(TEXT("Client: OnPostLogin called for: %s"), *NewPlayer->GetName()));
        }else
        {
            GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Blue, FString::Printf(TEXT("Server: OnPostLogin called for: %s"), *NewPlayer->GetName()));
        }
    }
}
