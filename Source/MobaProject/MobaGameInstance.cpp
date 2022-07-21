// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaGameInstance.h"
#include "AbilitySystemGlobals.h"

void UMobaGameInstance::OnStart()
{
    UAbilitySystemGlobals::Get().InitGlobalData();
}
