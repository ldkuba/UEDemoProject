// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MobaUnit.h"
#include "MobaController.generated.h"

/**
 * 
 */
UCLASS()
class MOBAPROJECT_API AMobaController : public APlayerController
{
	GENERATED_BODY()

public:
	AMobaController();

	virtual void PlayerTick(float DeltaTime) override;

	void SetPlayerName(const FString& NewName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controlls")
	float EdgePanBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controlls")
	float EdgePanSpeed;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void OnViewportResized(FViewport* Viewport, uint32);
	FIntPoint ViewportSize;

	void CameraEdgePan(float DeltaTime);

};
