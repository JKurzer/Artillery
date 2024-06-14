// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include <unordered_map>
#include "ArtilleryDispatch.h"
#include "FGunKey.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"
#include "UArtilleryAbilityMinimum.h"
#include "FArtilleryGun.generated.h"

/**
 * This class will be a data-driven instance of a gun that encapsulates a generic structured ability,
 * then exposes bindings for the phases of that ability as a component to be bound as specific gameplay abilities.
 *
 * 
 * Artillery gun is a not a UObject. This allows us to safely interact with it off the game thread.
 * Triggering the abilities is likely a no-go off the thread, but we can modify the attributes as needed.
 * This allows us to do some very powerful stuff to ensure that we always have the most up to date data.
 * Some dark things.
 */
USTRUCT(BlueprintType)
struct ARTILLERYRUNTIME_API FArtilleryGun
{
	GENERATED_BODY()

public:
	// this can be handed into abilities.
	friend class UArtilleryPerActorAbilityMinimum;
	FGunKey MyGunKey;
	


	/// <summary>
	/// activator goes here. called from fire control machine.
	/// should pass GunKey into the Ability Sequence.
	/// </summary>

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UArtilleryPerActorAbilityMinimum> Prefire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UArtilleryPerActorAbilityMinimum> PrefireCosmetic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UArtilleryPerActorAbilityMinimum> Fire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UArtilleryPerActorAbilityMinimum> FireCosmetic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UArtilleryPerActorAbilityMinimum> PostFire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UArtilleryPerActorAbilityMinimum> PostFireCosmetic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UArtilleryPerActorAbilityMinimum> FailedFireCosmetic;

	//we use the GunBinder delegate to link the MECHANICAL abilities to phases.
	//cosmetics don't get linked.
	//All other guns must name themselves. This one, the basal instance?
	//We know it. We have known it. We continue to know it.
	//See you soon, Chief.
	FArtilleryGun()
		: MyGunKey(UArtilleryDispatch::GetNewGunInstance("M6D Pistol"))
	{
		//assign gunkey
		Prefire->MyGunKey = MyGunKey;
		PrefireCosmetic->MyGunKey = MyGunKey;
		Fire->MyGunKey = MyGunKey;
		FireCosmetic->MyGunKey = MyGunKey;
		PostFire->MyGunKey = MyGunKey;
		PostFireCosmetic->MyGunKey = MyGunKey;
		FailedFireCosmetic->MyGunKey = MyGunKey;
	};


	//I'm sick and tired of the endless layers of abstraction.
	//Here's how it works. we fire the abilities from the gun.
	//OnGameplayAbilityEnded doesn't actually let you know if the ability was canceled.
	//That's... not so good. We use OnGameplayAbilityEndedWithData instead.
	void PreFireGun(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData = nullptr,
		bool RerunDueToReconcile = false,
		int DallyFramesToOmit = 0)
	{
		// Delegate signature:
		// DECLARE_DELEGATE_FiveParamsFArtilleryAbilityStateAlert,
		// FArtilleryStates,
		// int,
		// const FGameplayAbilityActorInfo*,
		// const FGameplayAbilityActivationInfo,
		// const FGameplayEventData* 
		Prefire->GunBinder.BindRaw(this, &FArtilleryGun::FireGun, RerunDueToReconcile, TriggerEventData);
		Fire->GunBinder.BindRaw(this, &FArtilleryGun::PostFireGun, RerunDueToReconcile, TriggerEventData);
		Prefire->CallActivateAbility(FGameplayAbilitySpecHandle(), ActorInfo, ActivationInfo, nullptr, TriggerEventData);
		if (!RerunDueToReconcile)
		{
			PrefireCosmetic->CallActivateAbility(Handle, ActorInfo, ActivationInfo, nullptr, TriggerEventData);
		}
	};

	/**************************************
	 *the following are delegates for Ability Minimum.
	 * We could likely use this with some cleverness to avoid object alloc while still getting
	 * per instance behavior but atm, that's not something I'm building.
	 * Dally frames don't work yet. But they will. Be ready.
	 *
	 * These are fired during end ability.
	 *****************************************
	 */

	/* This fires the gun when the prefire ability succeeds.
 * It will be tempting to reorder the parameters. Don't do this.
 * Again, this is a delegate function and the parametric order is what enables payloading.
 */
	void FireGun(
		FArtilleryStates OutcomeStates,
		int DallyFramesToOmit,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool RerunDueToReconcile,
		const FGameplayEventData* TriggerEventData) const
	{
		if (OutcomeStates == FArtilleryStates::Fired)
		{
			Fire->CallActivateAbility(FGameplayAbilitySpecHandle(), ActorInfo, ActivationInfo, nullptr,
			                          TriggerEventData);
			//TODO: BUILD CORRECT HANDLE HANDLING. HANDLES ARE OUR TICKET OUT OF THIS JOINT.
			if (!RerunDueToReconcile)
			{
				//TODO: BUILD CORRECT HANDLE HANDLING. HANDLES ARE OUR TICKET OUT OF THIS JOINT.
				FireCosmetic->CallActivateAbility(FGameplayAbilitySpecHandle(), ActorInfo, ActivationInfo, nullptr,
				                                  TriggerEventData);
			}
		}
		else
		{
			if (!RerunDueToReconcile)
			{
				//TODO: BUILD CORRECT HANDLE HANDLING. HANDLES ARE OUR TICKET OUT OF THIS JOINT.
				FailedFireCosmetic->CallActivateAbility(FGameplayAbilitySpecHandle(), ActorInfo, ActivationInfo,
				                                        nullptr, TriggerEventData);
			}
		}
	};

	void PostFireGun(
		FArtilleryStates OutcomeStates,
		int DallyFramesToOmit,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool RerunDueToReconcile,
		const FGameplayEventData* TriggerEventData) const
	{
		if (OutcomeStates == FArtilleryStates::Fired)
		{
			PostFire->CallActivateAbility(FGameplayAbilitySpecHandle(), ActorInfo, ActivationInfo, nullptr,
			                              TriggerEventData);
			//TODO: BUILD CORRECT HANDLE HANDLING. HANDLES ARE OUR TICKET OUT OF THIS JOINT.
			if (!RerunDueToReconcile)
			{
				//TODO: BUILD CORRECT HANDLE HANDLING. HANDLES ARE OUR TICKET OUT OF THIS JOINT.
				PostFireCosmetic->CallActivateAbility(FGameplayAbilitySpecHandle(), ActorInfo, ActivationInfo, nullptr,
				                                      TriggerEventData);
			}
		}
		else
		{
			if (!RerunDueToReconcile)
			{
				//TODO: BUILD CORRECT HANDLE HANDLING. HANDLES ARE OUR TICKET OUT OF THIS JOINT.
				FailedFireCosmetic->CallActivateAbility(FGameplayAbilitySpecHandle(), ActorInfo, ActivationInfo,
				                                        nullptr, TriggerEventData);
			}
		}
	};
};
