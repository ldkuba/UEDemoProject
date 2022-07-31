// Copyright Epic Games, Inc. All Rights Reserved.

#include "MobaProjectGameModeBase.h"
#include "MobaGameState.h"
#include "MobaPlayerState.h"

AMobaProjectGameModeBase::AMobaProjectGameModeBase()
    :Super()
{
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.bCanEverTick = true;

    DefaultCharacter = TSubclassOf<AMobaUnit>(AMobaUnit::StaticClass());
}

void AMobaProjectGameModeBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if(!IsNetMode(ENetMode::NM_Client))
    {
        AMobaGameState* gameState = GetGameState<AMobaGameState>();
        if(gameState)
        {
            FScore score = gameState->GetScore();
            gameState->SetScore({ score.Player1Score + 1, score.Player2Score + 1});
        }
    }
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

        AActor* playerStart = FindPlayerStart(NewPlayer);

        AMobaUnit* playerUnit = GetWorld()->SpawnActor<AMobaUnit>(
            DefaultCharacter,
            playerStart ? playerStart->GetActorLocation() : FVector(200.0f, 200.0f, 100.0f),
            playerStart ? playerStart->GetActorRotation() : FRotator::ZeroRotator,
            params
        );

        // On server save reference to player controller
        playerUnit->SetOwningPlayerController(Cast<APlayerController>(NewPlayer));

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
