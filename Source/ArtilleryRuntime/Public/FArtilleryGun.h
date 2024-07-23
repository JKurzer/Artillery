// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include <unordered_map>
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
 *
 * Ultimately, we'll need something like https://github.com/facebook/folly/blob/main/folly/concurrency/ConcurrentHashMap.h
 * if we want to get serious about this.
 */
USTRUCT(BlueprintType)
struct ARTILLERYRUNTIME_API FArtilleryGun
{
	GENERATED_BODY()

public:
	// this can be handed into abilities.
	friend class UArtilleryPerActorAbilityMinimum;
	FGunKey MyGunKey;

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
	//cosmetics don't get linked the same way.
	FArtilleryGun(const FGunKey& KeyFromDispatch)
	{
		MyGunKey = KeyFromDispatch;

		//assign gunkey
		Prefire->MyGunKey = MyGunKey;
		PrefireCosmetic->MyGunKey = MyGunKey;
		Fire->MyGunKey = MyGunKey;
		FireCosmetic->MyGunKey = MyGunKey;
		PostFire->MyGunKey = MyGunKey;
		PostFireCosmetic->MyGunKey = MyGunKey;
		FailedFireCosmetic->MyGunKey = MyGunKey;
	};

	virtual ~FArtilleryGun()
	{}

	//I'm sick and tired of the endless layers of abstraction.
	//Here's how it works. we fire the abilities from the gun.
	//OnGameplayAbilityEnded doesn't actually let you know if the ability was canceled.
	//That's... not so good. We use OnGameplayAbilityEndedWithData instead.
	virtual void PreFireGun(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData = nullptr,
		bool RerunDueToReconcile = false,
		int DallyFramesToOmit = 0)
	{
		// Delegate type:
		// DECLARE_DELEGATE_FiveParams FArtilleryAbilityStateAlert
		Prefire->GunBinder.BindRaw(this, &FArtilleryGun::FireGun, RerunDueToReconcile, TriggerEventData, Handle);
		Fire->GunBinder.BindRaw(this, &FArtilleryGun::PostFireGun, RerunDueToReconcile, TriggerEventData, Handle);
		Prefire->CallActivateAbility(FGameplayAbilitySpecHandle(), ActorInfo, ActivationInfo, nullptr,
		                             TriggerEventData);
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
	virtual void FireGun(
		FArtilleryStates OutcomeStates,
		int DallyFramesToOmit,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool RerunDueToReconcile,
		const FGameplayEventData* TriggerEventData,
		FGameplayAbilitySpecHandle Handle) const
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

	virtual void PostFireGun(
		FArtilleryStates OutcomeStates,
		int DallyFramesToOmit,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool RerunDueToReconcile,
		const FGameplayEventData* TriggerEventData,
		FGameplayAbilitySpecHandle Handle) const
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

	virtual void FArtilleryGunRebind(const FGunKey& KeyFromDispatch)
	{
		MyGunKey = KeyFromDispatch;
		//assign gunkey
		Prefire->MyGunKey = MyGunKey;
		PrefireCosmetic->MyGunKey = MyGunKey;
		Fire->MyGunKey = MyGunKey;
		FireCosmetic->MyGunKey = MyGunKey;
		PostFire->MyGunKey = MyGunKey;
		PostFireCosmetic->MyGunKey = MyGunKey;
		FailedFireCosmetic->MyGunKey = MyGunKey;
	}

	FArtilleryGun()
	{
		MyGunKey = Default;
	}

private:
	//Our debug value remains M6D.
	static const inline FGunKey Default = FGunKey("M6D", UINT64_MAX);
};
