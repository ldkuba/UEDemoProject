// Fill out your copyright notice in the Description page of Project Settings.


#include "MobaUnit.h"
#include "Net/UnrealNetwork.h"
#include "MobaController.h"
#include "MobaGameplayAbility.h"

// Sets default values
AMobaUnit::AMobaUnit()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void AMobaUnit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMobaUnit, UnitName);
}

APlayerController* AMobaUnit::GetOwningPlayerController() const
{
	if(HasAuthority())
	{
		return OwningPlayerController;
	}else
	{
		AMobaController* localController = GetWorld()->GetFirstPlayerController<AMobaController>();
		return (localController->GetPlayerUnit() == this) ? localController : nullptr;
	}
}

void AMobaUnit::SetOwningPlayerController(APlayerController* OwnerController)
{
	if(HasAuthority())
	{
		OwningPlayerController = OwnerController;
	}
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

// Abilities

void AMobaUnit::UseAbility(EMobaAbilityType AbilityType, EMobaMagicElement MagicElement)
{
	checkf(GetNetMode() != ENetMode::NM_Client, TEXT("UseAbility can only be called on server"));

	if(!IsValid(AbilitySystemComponent))
		return;

	FGameplayTagContainer AbilityTags = GetAbilityTags(AbilityType, MagicElement);
	AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTags);
}

void AMobaUnit::ConfirmAbility(EMobaAbilityType AbilityType, EMobaMagicElement MagicElement)
{
	checkf(GetNetMode() != ENetMode::NM_Client, TEXT("UseAbility can only be called on server"));

	if(!IsValid(AbilitySystemComponent))
		return;

	FGameplayTagContainer AbilityTags = GetAbilityTags(AbilityType, MagicElement);

	// Find ability with the given tags
	TArray<FGameplayAbilitySpecHandle> abilityHandles;
	AbilitySystemComponent->FindAllAbilitiesWithTags(abilityHandles, AbilityTags, true);
	if(abilityHandles.Num() != 1)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find ability with tags %s or more than 1 ability found"), *AbilityTags.ToString());
		return;
	}

	// Get ability spec and check if it is currently active
	FGameplayAbilitySpec* abilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(abilityHandles[0]);
	if(!abilitySpec->IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("Ability is not active - cannot confirm"));
		return;
	}

	// Confirm the cast for this ability
	UMobaGameplayAbility* ability = Cast<UMobaGameplayAbility>(abilitySpec->GetPrimaryInstance());
	if(!IsValid(ability))
	{
		UE_LOG(LogTemp, Error, TEXT("Could not cast ability to UMobaGameplayAbility"));
		return;
	}

	ability->ConfirmCast();
}

FGameplayTagContainer AMobaUnit::GetAbilityTags(EMobaAbilityType AbilityType, EMobaMagicElement MagicElement) const
{
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

	return AbilityTags;
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
