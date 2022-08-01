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
}

void AMobaProjectGameModeBase::SpawnPlayerUnit(AMobaController* PlayerController)
{
    AMobaPlayerState* playerState = PlayerController->GetPlayerState<AMobaPlayerState>();
    if(playerState)
    {
        // Can only spawn pawns on server
        checkf(!IsNetMode(ENetMode::NM_Client), TEXT("SpawnPlayerCharacter() can only be called on the server"));

        FActorSpawnParameters params;
        params.Owner = PlayerController;

        APawn* pawn = PlayerController->GetPawn();

        AActor* playerStart = FindPlayerStart(PlayerController, PlayerController->PlayerStartTag);

        AMobaUnit* playerUnit = GetWorld()->SpawnActor<AMobaUnit>(
            DefaultCharacter,
            playerStart ? playerStart->GetActorLocation() : FVector(200.0f, 200.0f, 100.0f),
            playerStart ? playerStart->GetActorRotation() : FRotator::ZeroRotator,
            params
        );

        pawn->SetActorLocation(playerStart->GetActorLocation());
        pawn->SetActorRotation(FRotator::ZeroRotator);

        // On server save reference to player controller
        playerUnit->SetOwningPlayerController(Cast<APlayerController>(PlayerController));
        playerUnit->SetUnitName(playerState->GetPlayerName());

        // TODO: change to AddPlayerUnit() to handle multiple controllable units
        playerState->SetPlayerUnit(playerUnit);
        PlayerController->OnPlayerUnitChanged(playerUnit);
    }
}

void AMobaProjectGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    AMobaController* mobaController = Cast<AMobaController>(NewPlayer);
    if(mobaController)
    {
        mobaController->PlayerStartTag = FString::Printf(TEXT("Player%d"), GetNumPlayers());
    }

    // Check if game can be started
    if(GetNumPlayers() >= MAX_PLAYERS)
    {
        RestartGame();
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

void AMobaProjectGameModeBase::RestartGame()
{
    checkf(!IsNetMode(ENetMode::NM_Client), TEXT("RestartGame() can only be called on the server"));

    AMobaGameState* gameState = GetGameState<AMobaGameState>();
    if(gameState)
    {
        gameState->SetScore({ 0, 0 });
    }

    StartNewRound();
}

void AMobaProjectGameModeBase::StartNewRound()
{
    checkf(!IsNetMode(ENetMode::NM_Client), TEXT("StartNewRound() can only be called on the server"));

    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PlayerController = Iterator->Get();
        if(!PlayerController)
            continue;

        AMobaController* mobaController = Cast<AMobaController>(PlayerController);
        if(mobaController)
        {
            AMobaUnit* playerUnit = mobaController->GetPlayerUnit();
            if(playerUnit)
            {
                playerUnit->Destroy();
            }

            SpawnPlayerUnit(mobaController);
        }
    }
}
