// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaUnitAttributeSet.h"
#include "MobaUnit.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UMobaUnitAttributeSet::UMobaUnitAttributeSet()
{
    MaxHealth = 100.0f;
}

void UMobaUnitAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UMobaUnitAttributeSet, Health, COND_None, REPNOTIFY_Always);
}

void UMobaUnitAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UMobaUnitAttributeSet, Health, OldValue);

    // Client side notify
    AMobaUnit* unit = Cast<AMobaUnit>(GetOwningActor());
    if(IsValid(unit))
    {
        unit->OnChangeUnitHealth(Health);
    }
}

void UMobaUnitAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData &Data) 
{
    Super::PostGameplayEffectExecute(Data);

    if (Data.EvaluatedData.Attribute == GetHealthAttribute()) 
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, MaxHealth));
        
        // Server side notify
        AMobaUnit* unit = Cast<AMobaUnit>(GetOwningActor());
        if(IsValid(unit))
        {
            unit->OnChangeUnitHealth(Health);
        }
    }
}
