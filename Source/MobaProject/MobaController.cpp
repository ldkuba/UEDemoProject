// Fill out your copyright notice in the Description page of Project Settings.

#include "MobaController.h"

#include "AIController.h"
#include "GameFramework/PlayerState.h"
#include "MobaGameInstance.h"
#include "MobaPlayerState.h"

void AMobaController::BeginPlay()
{
    Super::BeginPlay();

    // Setup callback
	FViewport::ViewportResizedEvent.AddUObject(this, &AMobaController::OnViewportResized);
	// Get current value
	GetViewportSize(ViewportSize.X, ViewportSize.Y);

    // Send player name to server
    if(IsLocalController())
    {
        UMobaGameInstance* gameInstance = Cast<UMobaGameInstance>(GetGameInstance());
        if(gameInstance)
        {
            ServerChangeName(gameInstance->PlayerName);
        }
    }
}

void AMobaController::OnViewportResized(FViewport* Viewport, uint32)
{
    ULocalPlayer* LocPlayer = Cast<ULocalPlayer>(Player);
    if(!LocPlayer)
        return;

	if (LocPlayer->ViewportClient->Viewport == Viewport)
    {
	    ViewportSize = Viewport->GetSizeXY();
    }
}

void AMobaController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);

    if(IsLocalPlayerController())
    {
        CameraEdgePan(DeltaTime);

        // Handle player movement
        if(WasInputKeyJustPressed(EKeys::RightMouseButton))
        {
            FVector HitLocation = FVector::ZeroVector;
            FHitResult Hit;
            GetHitResultUnderCursor(ECC_Visibility, true, Hit);
            HitLocation = Hit.Location;

            ServerMoveRequest(HitLocation);
        }else if(WasInputKeyJustPressed(EKeys::S))
        {
            ServerStopRequest();
        }
    }
}

void AMobaController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Setup callback
    InputComponent->BindAction<FSpellInputDelegate>("Offensive Spell", IE_Pressed, this, &AMobaController::UseAbility, EMobaAbilityType::Offensive);
    InputComponent->BindAction<FSpellInputDelegate>("Defensive Spell", IE_Pressed, this, &AMobaController::UseAbility, EMobaAbilityType::Defensive);
    InputComponent->BindAction<FSpellInputDelegate>("Special Spell", IE_Pressed, this, &AMobaController::UseAbility, EMobaAbilityType::Special);

    InputComponent->BindAction<FSpellInputDelegate>("Offensive Spell", IE_Released, this, &AMobaController::ConfirmAbility, EMobaAbilityType::Offensive);
    InputComponent->BindAction<FSpellInputDelegate>("Defensive Spell", IE_Released, this, &AMobaController::ConfirmAbility, EMobaAbilityType::Defensive);
    InputComponent->BindAction<FSpellInputDelegate>("Special Spell", IE_Released, this, &AMobaController::ConfirmAbility, EMobaAbilityType::Special);
}

// Ability inputing
void AMobaController::UseAbility(EMobaAbilityType AbilityType)
{
    ServerUseAbility(AbilityType);
}

void AMobaController::ServerUseAbility_Implementation(EMobaAbilityType AbilityType)
{
    AMobaPlayerState* playerState = GetPlayerState<AMobaPlayerState>();
    if(!playerState)
        return;

    AMobaUnit* playerUnit = GetPlayerUnit();
    if(!playerUnit)
        return;

    playerUnit->UseAbility(AbilityType, playerState->GetMagicElement());
}

void AMobaController::ConfirmAbility(EMobaAbilityType AbilityType)
{
    ServerConfirmAbility(AbilityType);
}

void AMobaController::ServerConfirmAbility_Implementation(EMobaAbilityType AbilityType)
{
    AMobaPlayerState* playerState = GetPlayerState<AMobaPlayerState>();
    if(!playerState)
        return;

    AMobaUnit* playerUnit = GetPlayerUnit();
    if(!playerUnit)
        return;

    playerUnit->ConfirmAbility(AbilityType, playerState->GetMagicElement());
}


AMobaUnit* AMobaController::GetPlayerUnit()
{
    AMobaPlayerState* playerState = GetPlayerState<AMobaPlayerState>();
    if(!playerState)
    {
        UE_LOG(LogTemp, Error, TEXT("Player state not found"));
        return nullptr;
    }

    return playerState->GetPlayerUnit();
}

void AMobaController::ServerMoveRequest_Implementation(FVector Destination)
{
    // TODO: make this fetch all currently selected units and issue move command to all
    AMobaUnit* playerUnit = GetPlayerUnit();
    if(!playerUnit)
    {
        UE_LOG(LogTemp, Error, TEXT("Player unit not found"));
        return;
    }

    // Direct the Pawn towards that location
    AAIController* aiController = playerUnit->GetController<AAIController>();
    if(aiController)
    {
        aiController->MoveToLocation(Destination, 1.0f, false, true, true, false);
    }
}

void AMobaController::ServerStopRequest_Implementation()
{
    AMobaUnit* playerUnit = GetPlayerUnit();
    if(!playerUnit)
    {
        UE_LOG(LogTemp, Error, TEXT("Player unit not found"));
        return;
    }

    // Stop the pawn
    AAIController* aiController = playerUnit->GetController<AAIController>();
    if(aiController)
    {
        aiController->StopMovement();
    }
}

void AMobaController::CameraEdgePan(float DeltaTime)
{
    APawn* pawn = GetPawn();
    if(!pawn)
        return;

    ULocalPlayer* LocPlayer = Cast<ULocalPlayer>(Player);
	if (!LocPlayer->ViewportClient->Viewport || !LocPlayer->ViewportClient->Viewport->IsForegroundWindow())
	{
		// viewport is either not present or not in the foreground.
        return;
	}

    float mousePosX;
    float mousePosY;
    
    GetMousePosition(mousePosX, mousePosY);
    mousePosX = FMath::Clamp(mousePosX, 0, ViewportSize.X);
    mousePosY = FMath::Clamp(mousePosY, 0, ViewportSize.Y);

    // UE_LOG(LogTemp, Log, TEXT("Mouse Position: %f, %f"), mousePosX, mousePosY);

    FVector cameraMotion = FVector::ZeroVector;    
    if(mousePosX < EdgePanBorder)
    {
        cameraMotion.Y = -1;
    }else if(mousePosX > ViewportSize.X - EdgePanBorder)
    {
        cameraMotion.Y = 1;
    }
    
    if(mousePosY < EdgePanBorder)
    {
        cameraMotion.X = 1;
    }else if(mousePosY > ViewportSize.Y - EdgePanBorder)
    {
        cameraMotion.X = -1;
    }

    pawn->AddActorWorldOffset(cameraMotion.GetSafeNormal() * EdgePanSpeed * DeltaTime);
}
