// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include <unordered_map>

#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"
#include "UArtilleryAbilityMinimum.h"
#include "UArtilleryAbilitySequence.h"
#include "FArtilleryGun.generated.h"

/**
 * This class will be a data-driven instance of a gun that encapsulates a generic structured ability,
 * then exposes bindings for the phases of that ability as a component to be bound as specific gameplay abilities.
 *
 * This is still mostly a sketch just to get some code on deck that expresses the idea and start fiddling with it
 * to see how it might feel to develop using it.
 * 
 * https://github.com/lucoiso/UEElementusModules/tree/main/Modules/Source/ElementusAbilitySystem/Public
 * is prolly the thing to harvest.
 * 
 * Artillery gun is a not a UObject. This allows us to safely interact with it off the game thread.
 * Triggering the abilities is likely a no-go off the thread, but we can modify the attributes as needed.
 * This allows us to do some very very powerful stuff to ensure that we always have the most up to date data.
 * Some dark things.
 */
USTRUCT(BlueprintType)
struct ARTILLERYRUNTIME_API FArtilleryGun 
{

	GENERATED_BODY()

public:	
		
		// this can be handed into abilities.
		FGunKey MyGunKey;
		
		/// <summary>
		/// activator goes here. called from fire control machine.
		/// should pass GunKey into the Ability Sequence.
		/// </summary>

		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<UArtilleryPerActorAbilityMinimum> Prefire;

		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<UArtilleryPerActorAbilityMinimum> CosmeticPrefire;

		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<UArtilleryPerActorAbilityMinimum> Fire;
		
		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<UArtilleryPerActorAbilityMinimum> CosmeticFire;
		
		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<UArtilleryPerActorAbilityMinimum> PostFire;
		
		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<UArtilleryPerActorAbilityMinimum> PostFireCosmetic;

		//stop execution BP node needed here
		UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Latency Hiding")
		int DallyFramesToOmit = 0;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Latency Hiding")
		bool RerunDueToReconcile = false;

		FArtilleryGun()
		{
			//assign gunkey
			Prefire->MyGunKey = MyGunKey;
			CosmeticPrefire->MyGunKey = MyGunKey;
			Fire->MyGunKey = MyGunKey;
			CosmeticFire->MyGunKey = MyGunKey;
			PostFire->MyGunKey = MyGunKey;
			PostFireCosmetic->MyGunKey = MyGunKey;
		};

protected:
	TObjectPtr <UArtilleryAbilitySequence> EncapsulatedGASMachinery;
};

