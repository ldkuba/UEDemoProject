// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MobaUnit.generated.h"

UCLASS()
class MOBAPROJECT_API AMobaUnit : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMobaUnit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_UnitName, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FText UnitName;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void SetUnitName(const FString& NewName);

	UFUNCTION()
	void OnRep_UnitName();

	UFUNCTION(BlueprintImplementableEvent)
	void OnChangeUnitName(const FText& NewName);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
