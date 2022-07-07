// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaUnit.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AMobaUnit::AMobaUnit()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AMobaUnit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMobaUnit, UnitName);
}

// Called when the game starts or when spawned
void AMobaUnit::BeginPlay()
{
	Super::BeginPlay();
	
	if(GetNetMode() == ENetMode::NM_Client)
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Green, FString::Printf(TEXT("%s: Unit spawned on client: %s"), *GetName(), *UnitName.ToString()));
	}
}

// Called every frame
void AMobaUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMobaUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMobaUnit::SetUnitName(const FString& NewName)
{
	UnitName = FText::FromString(NewName);
	OnRep_UnitName();
}

void AMobaUnit::OnRep_UnitName()
{
	if(GetNetMode() == ENetMode::NM_Client)
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Green, FString::Printf(TEXT("%s: Unit name replicated on client: %s"), *GetName(), *UnitName.ToString()));
	}

	// Call blueprint method
	OnChangeUnitName(UnitName);
}
