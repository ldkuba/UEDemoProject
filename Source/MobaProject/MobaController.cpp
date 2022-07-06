// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaController.h"

AMobaController::AMobaController()
    :Super()
{
    DefaultCharacter = TSubclassOf<ACharacter>(ACharacter::StaticClass());
}

void AMobaController::BeginPlay()
{
    Super::BeginPlay();

    // Setup callback
	FViewport::ViewportResizedEvent.AddUObject(this, &AMobaController::OnViewportResized);
	// Get current value
	GetViewportSize(ViewportSize.X, ViewportSize.Y);

    ENetMode mode = GetNetMode();
    if(mode == ENetMode::NM_ListenServer || mode == ENetMode::NM_DedicatedServer)
    {
        if(GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Blue, TEXT("Server: Spawning pawn for player"));
        SpawnPlayerCharacter();
    }
}

void AMobaController::SpawnPlayerCharacter()
{
    APawn* pawn = GetPawn();

    FActorSpawnParameters params;
    params.Owner = this;

    ControlledCharacter = GetWorld()->SpawnActor<ACharacter>(
        DefaultCharacter,
        pawn ? pawn->GetActorLocation() : FVector(200.0f, 200.0f, 100.0f),
        pawn ? pawn->GetActorRotation() : FRotator::ZeroRotator,
        params
    );

    // ControlledCharacter->AddActorWorldOffset(FVector(200.0f, 0.0f, 0.0f));
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
