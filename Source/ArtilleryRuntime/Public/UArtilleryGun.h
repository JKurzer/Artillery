// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <unordered_map>

#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"
#include "UArtilleryGun.generated.h"

/**
 * This class will be a data-driven instance of a gun that encapsulates a generic structured ability,
 * then exposes bindings for the phases of that ability as a component to be bound as specific gameplay abilities.
 *
 * This is still mostly a sketch just to get some code on deck that expresses the idea and start fiddling with it
 * to see how it might feel to develop using it.
 * 
 * https://github.com/lucoiso/UEElementusModules/tree/main/Modules/Source/ElementusAbilitySystem/Public
 * is prolly the thing to harvest.
 */

UCLASS()
class ARTILLERYRUNTIME_API UArtilleryGun : public UActorComponent
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

private:
	TObjectPtr <UArtilleryAbilitySequence> EncapsulatedGASMachinery;
};
