// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaAbilityTargetActor.h"
#include "Abilities/GameplayAbility.h"
#include "Net/UnrealNetwork.h"
#include "MobaController.h"

AMobaAbilityTargetActor::AMobaAbilityTargetActor(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
	ShouldProduceTargetDataOnServer = false;
    bReplicates = true;

    OwningPlayerController = nullptr;
}

bool AMobaAbilityTargetActor::ShouldProduceTargetData() const
{
    return true;
}

void AMobaAbilityTargetActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMobaAbilityTargetActor, OwningPlayerController);
}

void AMobaAbilityTargetActor::BeginPlay()
{
    Super::BeginPlay();
}

void AMobaAbilityTargetActor::StartTargeting(UGameplayAbility* InAbility)
{
	Super::StartTargeting(InAbility);
	SourceActor = InAbility->GetCurrentActorInfo()->AvatarActor.Get();

    // This only runs on server
    if(!IsValid(OwningPlayerController))
    {
        AMobaUnit* mobaUnit = Cast<AMobaUnit>(SourceActor);
        OwningPlayerController = Cast<AMobaController>(mobaUnit->GetOwningPlayerController());
        if(!IsValid(OwningPlayerController))
        {
            return;
        }
    }

    SetOwner(OwningPlayerController);
}

void AMobaAbilityTargetActor::ConfirmTargetingAndContinue()
{
    if(GetNetMode() != ENetMode::NM_Client)
    {
        UE_LOG(LogTemp, Error, TEXT("ConfirmTargetingAndContinue() called on server"));
    }else
    {
        UE_LOG(LogTemp, Error, TEXT("ConfirmTargetingAndContinue() called on client"));
    }

    // If this is a unit owned by the listening server, do calculations immediatly
    if(OwningPlayerController->IsLocalController())
    {
        ServerSendTargetingInput(CalculateTargetData());
    }else
    {
        ClientConfirmInput();
    }
}

void AMobaAbilityTargetActor::ClientConfirmInput_Implementation()
{
    if(GetNetMode() != ENetMode::NM_Client)
    {
        UE_LOG(LogTemp, Error, TEXT("ClientConfirmInput_Implementation() called on server"));
    }else
    {
        UE_LOG(LogTemp, Error, TEXT("ClientConfirmInput_Implementation() called on client"));
    }

    // Send client targeting data to server
    if(OwningPlayerController)
    {
        ServerSendTargetingInput(CalculateTargetData());
    }
}

FGameplayAbilityTargetDataHandle AMobaAbilityTargetActor::CalculateTargetData()
{
    // Get mouse world position from
    FVector HitLocation = FVector::ZeroVector;
    FHitResult Hit;
    OwningPlayerController->GetHitResultUnderCursor(ECC_Visibility, true, Hit);
    HitLocation = Hit.Location;
    
    FGameplayAbilityTargetDataHandle Handle;
    FGameplayAbilityTargetData_LocationInfo* locationData = new FGameplayAbilityTargetData_LocationInfo();
    
    FGameplayAbilityTargetingLocationInfo targetLocationInfo;
    targetLocationInfo.LocationType = EGameplayAbilityTargetingLocationType::Type::LiteralTransform;
    targetLocationInfo.LiteralTransform = FTransform(HitLocation);
    locationData->TargetLocation = targetLocationInfo;
    
    Handle.Add(locationData);

    return Handle;
}

void AMobaAbilityTargetActor::ServerSendTargetingInput_Implementation(FGameplayAbilityTargetDataHandle Handle)
{
    if(GetNetMode() != ENetMode::NM_Client)
    {
        UE_LOG(LogTemp, Error, TEXT("ServerSendTargetingInput_Implementation() called on server"));
    }else
    {
        UE_LOG(LogTemp, Error, TEXT("ServerSendTargetingInput_Implementation() called on client"));
    }

    TargetDataReadyDelegate.Broadcast(Handle);
}
