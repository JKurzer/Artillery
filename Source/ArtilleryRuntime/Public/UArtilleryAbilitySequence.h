// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <unordered_map>

#include "GameplayEffectTypes.h"

#include "GameplayEffect.h"

#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbilitySpecHandle.h"

#include "Abilities/GameplayAbilityTypes.h"

#include "UArtilleryAbilitySequence.generated.h"

/**
 * This is more of a note with the idea embodied in code. Still not sure I love it, but...
 * I think the simplest way to fix a lot of our problems is to add two concepts to GAS:
 * ActivateLate and AtMostOnce GameplayCues to make sure things don't get re-fired.
 * 
 * By breaking down the ability into a series of events that are guaranteed to be called in a known order
 * and by characterizing those events as phases that do or do not allow dally trim, we can comfortably
 * make adding support for ActivateLate.
 * 
 * The phases of an Artillery ability are
 * 
 * [Try: fail, start]
 * Prefire
 * Prefire Cosmetic
 * 
 * [Activate]
 * Fire
 * Fire Cosmetic
 * 
 * [Outro]
 * PostFire
 * PostFire Cosmetic
 * 
 * 
 * Either Commit OR end may be called. This is all bound together in artillery gun, hiding the ability sequencing system by encapsulation.
 * 
 * This means that artillery abilities always follow a very fixed format. In fact, this format is quite hard to express
 * WITHOUT a fixed body of supporting machinery in GAS.
 * 
 * AtMostOnce:
 * To ensure gameplay cues only activate once, I think we can do just some pretty simple trickery.
 * All Abilities in artillery are required to parent from this class.
 * 
 */
UCLASS(NotBlueprintable)
class ARTILLERYRUNTIME_API UArtilleryAbilitySequence : public UArtilleryPerActorAbilityMinimum
{


	// --------------------------------------
	//	About Input
	// --------------------------------------
	// Artillery abilities do not ever get called by input directly, and are wholly divorced from the input that fired them.
	// We may use "input" as a way to pass some small amount of state, but at the moment, it's just not used at all. Instead,
	// Artillery abilities are fired by their parent Ability Sequence (this class) which is encapsulated in FArtilleryGun.
	// 
	// Allowing inputfulness to leak into individual abilities has a lot of positives, but for a deterministic system,
	// we judged the negatives to be too large - maintaining two activation code paths just didn't make sense.
	// 
	//little anxious about lines 68-70 in UGameplayAbility.
	GENERATED_BODY()

public:


	// --------------------------------------
	//	ActivateAbility
	// (Per the docs of GameplayAbility.h)
	// 
	// This is the bit that matters to us: When K2_ActivateAbility logically finishes, then we will expect Commit/End to have been called.
	// Interestingly, Ability Sequence itself really just reports back what its encapsulated abilities do.
	// --------------------------------------

	FGunKey MyGunKey;
	/**
	 * The main function that defines what an ability does.
	 *  -Child classes will want to override this
	 *  -This function graph should call CommitAbility
	 *  -This function graph should call EndAbility
	 *
	 *  Latent/async actions are ok in this graph. Note that Commit and EndAbility calling requirements speak to the K2_ActivateAbility graph.
	 *  In C++, the call to K2_ActivateAbility() may return without CommitAbility or EndAbility having been called. But it is expected that this
	 *  will only occur when latent/async actions are pending. When K2_ActivateAbility logically finishes, then we will expect Commit/End to have been called.
	 *
	 */
	void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Do boilerplate init stuff and then call ActivateAbility */
	void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData = nullptr) override;

	//native End
	void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;


	bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;


	/** Generates a GameplayEffectContextHandle from our owner and an optional TargetData.*/
	FGameplayEffectContextHandle GetContextFromOwner(FGameplayAbilityTargetDataHandle OptionalTargetData) const override;

	/** Returns an effect context, given a specified actor info */
	FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;

private:

	//these have no function in the ability sequence.
	void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override {};
	void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override {};
	/** Called from AbilityTask_WaitConfirmCancel to handle input confirming */
	void OnWaitingForConfirmInputBegin() override {};
	void OnWaitingForConfirmInputEnd() override {};

	/** Ability sequences, and artillery in general, make use of only cosmetic events and cues where possible. */
	void SendGameplayEvent(FGameplayTag EventTag, FGameplayEventData Payload) override {};
};
