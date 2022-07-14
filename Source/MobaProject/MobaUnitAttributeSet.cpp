// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaUnitAttributeSet.h"
#include "Net/UnrealNetwork.h"

UMobaUnitAttributeSet::UMobaUnitAttributeSet(){}

void UMobaUnitAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UMobaUnitAttributeSet, Health, COND_None, REPNOTIFY_Always);
}

void UMobaUnitAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaUnitAttributeSet, Health, OldValue);
}
