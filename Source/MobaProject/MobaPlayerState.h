// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MobaUnit.h"
#include "MobaPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MOBAPROJECT_API AMobaPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

	AMobaPlayerState();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void OnRep_PlayerName() override;
	
	inline void SetPlayerUnit(AMobaUnit* NewUnit) { ControlledCharacter = NewUnit; }
	inline AMobaUnit* GetPlayerUnit() const { return ControlledCharacter; }

	inline void SetMagicElement(EMobaMagicElement NewElement) { CurrentMagicElement = NewElement; }
	inline EMobaMagicElement GetMagicElement() const { return CurrentMagicElement; }

protected:
	// PlayerState hold a reference to the controlled character
	UPROPERTY(Replicated, BlueprintReadOnly)
	AMobaUnit* ControlledCharacter;

	UPROPERTY(Replicated, BlueprintReadOnly)
	EMobaMagicElement CurrentMagicElement;
};
