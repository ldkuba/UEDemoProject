// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaPlayerPawn.h"

// Sets default values
AMobaPlayerPawn::AMobaPlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMobaPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMobaPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMobaPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

