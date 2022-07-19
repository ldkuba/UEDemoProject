// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaUnit.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AMobaUnit::AMobaUnit()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
}

void AMobaUnit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMobaUnit, UnitName);
}

UAbilitySystemComponent* AMobaUnit::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Called when the game starts or when spawned
void AMobaUnit::BeginPlay()
{
	Super::BeginPlay();

	// Get pointer to attribute set
	if(IsValid(AbilitySystemComponent))
	{
		AttributeSet = AbilitySystemComponent->GetSet<UMobaUnitAttributeSet>();
	}

	// Give all default abilities
	if(HasAuthority())
	{
		for(TSubclassOf<UGameplayAbility>& Ability : DefaultAbilities)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1, INDEX_NONE, this));
		}
	}

	// This manually calls the callbacks with the initial values
	OnChangeUnitName(UnitName);
	OnChangeUnitHealth(AttributeSet->GetHealth());
}

// Called every frame
void AMobaUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMobaUnit::UseAbility(EMobaAbilityType AbilityType, EMobaMagicElement MagicElement)
{
	checkf(GetNetMode() != ENetMode::NM_Client, TEXT("UseAbility can only be called on server"));

	if(!IsValid(AbilitySystemComponent))
		return;

	FGameplayTagContainer AbilityTags;

	switch(AbilityType)
	{
		case EMobaAbilityType::Offensive:
			AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("AbilityType.Offensive"))));
			break;
		case EMobaAbilityType::Defensive:
			AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("AbilityType.Defensive"))));
			break;
		case EMobaAbilityType::Special:
			AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("AbilityType.Special"))));
			break;
		default:
			break;
	}

	switch(MagicElement)
	{
		case EMobaMagicElement::None:
			AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("MagicElement.None"))));
			break;
		case EMobaMagicElement::Fire:
			AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("MagicElement.Fire"))));
			break;
		case EMobaMagicElement::Water:
			AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("MagicElement.Water"))));
			break;
		case EMobaMagicElement::Wind:
			AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("MagicElement.Wind"))));
			break;
		default:
			break;
	}

	AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
}

void AMobaUnit::SetUnitName(const FString& NewName)
{
	UnitName = FText::FromString(NewName);
	OnRep_UnitName();
}

void AMobaUnit::OnRep_UnitName()
{
	// Call blueprint method
	OnChangeUnitName(UnitName);
}
