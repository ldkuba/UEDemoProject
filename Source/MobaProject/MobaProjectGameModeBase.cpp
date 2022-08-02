// Copyright Epic Games, Inc. All Rights Reserved.

#include "MobaProjectGameModeBase.h"
#include "MobaGameState.h"
#include "MobaPlayerState.h"

AMobaProjectGameModeBase::AMobaProjectGameModeBase()
    :Super()
{
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.bCanEverTick = true;

    bIsRoundInProgress = false;

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
        
        AActor* playerStart = FindPlayerStart(PlayerController, FString::Printf(TEXT("Player%d"), PlayerController->PlayerIndex));

        AMobaUnit* playerUnit = GetWorld()->SpawnActor<AMobaUnit>(
            DefaultCharacter,
            playerStart ? playerStart->GetActorLocation() : FVector(200.0f, 200.0f, 100.0f),
            playerStart ? playerStart->GetActorRotation() : FRotator::ZeroRotator,
            params
        );

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
        mobaController->PlayerIndex = GetNumPlayers();
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

    bIsRoundInProgress = true;
}

void AMobaProjectGameModeBase::HandleUnitDeath(AMobaUnit* Unit)
{
    if(!bIsRoundInProgress)
        return;

    bIsRoundInProgress = false;

    int32 looserID = Cast<AMobaController>(Unit->GetOwningPlayerController())->PlayerIndex;
    FString winnerName = "";

    // Find winners name
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AMobaController* PlayerController = Cast<AMobaController>(Iterator->Get());
        if(PlayerController)
        {
            if(PlayerController->PlayerIndex != looserID)
            {
                winnerName = PlayerController->GetPlayerState<AMobaPlayerState>()->GetPlayerName();
                break;
            }
        }
    }

    // Display round end message for all players
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        AMobaController* PlayerController = Cast<AMobaController>(Iterator->Get());
        if(PlayerController)
        {
            PlayerController->ToggleRoundEndScreen(true, winnerName);
        }
    }
    
    // Add one point to winner
    AMobaGameState* gameState = GetGameState<AMobaGameState>();
    FScore score = gameState->GetScore();
    if(looserID == 2)
    {
        gameState->SetScore({ score.Player1Score + 1, score.Player2Score });
    }else
    {
        gameState->SetScore({ score.Player1Score, score.Player2Score + 1 });
    }

    // Wait for 5 seconds then hide victory screens and restart round
    FTimerHandle timerHandle;
    GetWorldTimerManager().SetTimer(timerHandle, FTimerDelegate::CreateLambda([=]()
    {
        // Hide round end screen for all players
        for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
        {
            AMobaController* PlayerController = Cast<AMobaController>(Iterator->Get());
            if(PlayerController)
            {
                PlayerController->ToggleRoundEndScreen(false, "");
            }
        }

        // Restart round
        StartNewRound();
    }), 5.0f, false);
}
