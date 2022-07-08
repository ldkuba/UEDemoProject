// Fill out your copyright notice in the Description page of Project Settings.

#include "MobaController.h"

#include "Blueprint/AIBlueprintHelperLibrary.h"
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
        }
    }
}

void AMobaController::ServerMoveRequest_Implementation(FVector Destination)
{
    // TODO: make this fetch all currently selected units and issue move command to all
    AMobaPlayerState* playerState = GetPlayerState<AMobaPlayerState>();
    if(!playerState)
    {
        UE_LOG(LogTemp, Error, TEXT("Player state not found"));
        return;
    }

    AMobaUnit* playerUnit = playerState->GetPlayerUnit();
    if(!playerUnit)
    {
        UE_LOG(LogTemp, Error, TEXT("Player unit not found"));
        return;
    }

    if(GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Green, FString::Printf(TEXT("Server: %s - move request to: %s"), *(playerUnit->GetUnitName().ToString()), *Destination.ToString()));
    
        if(!playerUnit->GetController())
        {
            GEngine->AddOnScreenDebugMessage(-1, 100.0f, FColor::Red, FString::Printf(TEXT("Server: Controller for %s is null!"), *(playerUnit->GetUnitName().ToString())));
        }
    }

    // Direct the Pawn towards that location
    UAIBlueprintHelperLibrary::SimpleMoveToLocation(playerUnit->GetController(), Destination);
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
