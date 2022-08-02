// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "MobaUnitAttributeSet.h"
#include "MobaUnit.generated.h"

UENUM()
enum class EMobaAbilityType : uint8
{
	Offensive,
	Defensive,
	Special
};

UENUM()
enum class EMobaMagicElement : uint8
{
	None,
	Fire,
	Water,
	Wind
};

UENUM()
enum class EMobaAbilityTargetingType : uint8
{
	MouseWorldLocation,
	MouseDirection,
	TargetUnit
};

UCLASS()
class MOBAPROJECT_API AMobaUnit : public ACharacter, public IAbilitySystemInterface 
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMobaUnit();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Get the PlayerController belonging to the owner player of this unit.
	APlayerController* GetOwningPlayerController() const;
	void SetOwningPlayerController(APlayerController* OwnerController);

	// Default abilities
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> DefaultAbilities;

	void UseAbility(EMobaAbilityType AbilityType, EMobaMagicElement MagicElement);
	void ConfirmAbility(EMobaAbilityType AbilityType, EMobaMagicElement MagicElement);

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

	// Death
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDeath();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	FGameplayTagContainer GetAbilityTags(EMobaAbilityType AbilityType, EMobaMagicElement MagicElement) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Attributes")
	const UMobaUnitAttributeSet* AttributeSet;

	UPROPERTY(ReplicatedUsing = OnRep_UnitName, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FText UnitName;

	// Server reference to owning player controller
	APlayerController* OwningPlayerController;
};
