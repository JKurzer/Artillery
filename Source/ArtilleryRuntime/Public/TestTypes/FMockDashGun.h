 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include <unordered_map>

#include "ArtilleryDispatch.h"
#include "AssetTypeCategories.h"
#include "FGunKey.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"
#include "UArtilleryAbilityMinimum.h"
#include "FArtilleryGun.h"
#include "FMockDashGun.generated.h"




/**
 * This test class encapsulates a dash.
 * Finally.
 */
USTRUCT(BlueprintType)
struct ARTILLERYRUNTIME_API FMockDashGun : public FArtilleryGun 
{
	GENERATED_BODY()

public:
	// this can be handed into abilities.
	friend class UArtilleryPerActorAbilityMinimum;
	UArtilleryDispatch* MyDispatch;


	//this just sets the gunkey.
	//this mock doesn't use the delegate chaining, because it doesn't use abilities.
	FMockDashGun(const FGunKey& KeyFromDispatch)
	{
		MyGunKey = KeyFromDispatch;
	};


	virtual void PreFireGun(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData = nullptr,
		bool RerunDueToReconcile = false,
		int DallyFramesToOmit = 0) 
		override
	{
		auto bind =  MyDispatch->GetAttrib(MyProbableOwner, AttribKey::DashCharges);
		if(bind != std::nullopt && bind.value() > 0)
		{
			FireGun(FArtilleryStates::Fired, 0, ActorInfo, ActivationInfo, TriggerEventData, false, Handle);
		}
	};

	virtual void FireGun(
		FArtilleryStates OutcomeStates,
		int DallyFramesToOmit,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool RerunDueToReconcile,
		const FGameplayEventData* TriggerEventData,
		FGameplayAbilitySpecHandle Handle) const 
		override
	{
		
	};

	virtual void PostFireGun(
		FArtilleryStates OutcomeStates,
		int DallyFramesToOmit,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool RerunDueToReconcile,
		const FGameplayEventData* TriggerEventData,
		FGameplayAbilitySpecHandle Handle) const
		override
	{
			throw;
	};

	virtual bool Initialize(const FGunKey& KeyFromDispatch, bool MyCodeWillHandleKeys)
		override
	{
		MyDispatch = GWorld->GetSubsystem<UArtilleryDispatch>();
		return Super::Initialize(KeyFromDispatch, MyCodeWillHandleKeys);
	}

	FMockDashGun()
	{
		MyGunKey = Default;
	}
	private:
	//Our debug value remains M6D.
	static const inline FGunKey Default = FGunKey("M6D", UINT64_MAX);


};