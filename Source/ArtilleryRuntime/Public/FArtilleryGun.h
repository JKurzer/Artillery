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
		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<UArtilleryUninstancedAbilityMinimum> CosmeticPrefire;
		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<UArtilleryUninstancedAbilityMinimum> WindUp;
		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<UArtilleryUninstancedAbilityMinimum> Fire;
		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<UArtilleryUninstancedAbilityMinimum> CosmeticFire;
		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<UArtilleryUninstancedAbilityMinimum> WindDown;

		//stop execution BP node needed here
		UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Latency Hiding")
		int DallyFramesToOmit = 0;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Latency Hiding")
		bool RerunDueToReconcile = false;

protected:
	TObjectPtr <UArtilleryAbilitySequence> EncapsulatedGASMachinery;
};

