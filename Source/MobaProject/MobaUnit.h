// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "MobaUnitAttributeSet.h"
#include "MobaUnit.generated.h"

UCLASS()
class MOBAPROJECT_API AMobaUnit : public ACharacter, public IAbilitySystemInterface 
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMobaUnit();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Default abilities
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> DefaultAbilities;

	// Unit name
	UFUNCTION(BlueprintCallable)
	void SetUnitName(const FString& NewName);
	inline FText GetUnitName() const { return UnitName; }
	UFUNCTION()
	void OnRep_UnitName();

	// Blueprint callbacks
	UFUNCTION(BlueprintImplementableEvent)
	void OnChangeUnitName(const FText& NewName);
	UFUNCTION(BlueprintImplementableEvent)
	void OnChangeUnitHealth(const FGameplayAttributeData& NewHealth);
	UFUNCTION(BlueprintImplementableEvent)
	void OnUseAbilityWithTags(const FGameplayTagContainer& AbilityTags);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Attributes")
	const UMobaUnitAttributeSet* AttributeSet;

	UPROPERTY(ReplicatedUsing = OnRep_UnitName, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FText UnitName;
};
