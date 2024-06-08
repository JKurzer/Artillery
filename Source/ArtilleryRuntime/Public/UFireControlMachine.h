// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <unordered_map>

#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "FArtilleryGun.h"
#include "Abilities/GameplayAbility.h"
#include "Containers/CircularQueue.h"
#include <bitset>
#include "UAbilityFireControl.generated.h"

//dynamic constructed statemachine for matching patterns in action records to triggering abilities.

struct FActionBitMask
{
	std::bitset<14> buttons;
	std::bitset<6> events;
};

typedef uint64_t FGunKey;

class FActionPattern 
{
	virtual bool runPattern(FActionBitMask ToSeek) = 0;
};


//Generally, a firecontrolmachine represents a specific gun or set of guns
//for a specific actor. this, combined with the ACS, allows you to fully 
//integrate artillery with your existing GAS designs.
class ARTILLERYRUNTIME_API UFireControlMachine : public UActorComponent

{
public:	
		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TMap<FGunKey, FArtilleryGun> MyManagedGuns;
		UAttributeSet MyAttributes; // might want to defactor this to an ECS, but I actually quite like it here.
		UAbilitySystemComponent* SystemComponentToBind;
		bool pushPatternToRunner(FActionPattern ToBind, FActionBitMask ToSeek, FGunKey ToFire)
		{
			return false;
		};

		bool FireGun(GunKey GunID, bool InputAlreadyUsedOnce)
		{
			return false;
		};
		
		void BeginPlay() override {
			Super();
			//likely want to manage system component bind here by checking for actor parent.
			UActorComponent::GetOwner();
		};
};