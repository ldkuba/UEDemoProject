// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MobaUnit.h"

#include "MobaController.generated.h"

DECLARE_DELEGATE_OneParam(FSpellInputDelegate, const EMobaAbilityType);

class AMobaUnit;

/**
 * 
 */
UCLASS()
class MOBAPROJECT_API AMobaController : public APlayerController
{
	GENERATED_BODY()

public:

	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	void SetPlayerName(const FString& NewName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controlls")
	float EdgePanBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controlls")
	float EdgePanSpeed;

	UFUNCTION(BlueprintCallable)
	AMobaUnit* GetPlayerUnit();
	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerUnitChanged(AMobaUnit* NewUnit);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void UseAbility(EMobaAbilityType AbilityType);
	UFUNCTION(Server, Reliable)
	void ServerUseAbility(EMobaAbilityType AbilityType);

	void ConfirmAbility(EMobaAbilityType AbilityType);
	UFUNCTION(Server, Reliable)
	void ServerConfirmAbility(EMobaAbilityType AbilityType);

private:
	void OnViewportResized(FViewport* Viewport, uint32);
	FIntPoint ViewportSize;

	void CameraEdgePan(float DeltaTime);

	UFUNCTION(Server, Unreliable)
	void ServerMoveRequest(FVector Destination);

	UFUNCTION(Server, Unreliable)
	void ServerStopRequest();

};
