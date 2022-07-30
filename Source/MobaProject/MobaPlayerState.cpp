// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "MobaController.h"

AMobaPlayerState::AMobaPlayerState()
{
    CurrentMagicElement = EMobaMagicElement::None;
}

void AMobaPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMobaPlayerState, ControlledCharacter);
    DOREPLIFETIME(AMobaPlayerState, CurrentMagicElement);
}

void AMobaPlayerState::OnRep_PlayerName()
{
    Super::OnRep_PlayerName();

    if(GetNetMode() != ENetMode::NM_Client && ControlledCharacter)
    {
        ControlledCharacter->SetUnitName(GetPlayerName());
    }
}
