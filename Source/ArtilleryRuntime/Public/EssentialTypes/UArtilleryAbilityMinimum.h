// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <unordered_map>

#include "GameplayEffectTypes.h"

#include "Abilities/GameplayAbility.h"
#include "GameplayAbilitySpecHandle.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayEffect.h"
#include "FGunKey.h"

#include "GameplayTaskOwnerInterface.h"
#include "GameplayTask.h"
#include "UArtilleryAbilityMinimum.generated.h"

/**
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
 * Either Commit OR end may be called. EACH of these is an instance, per actor, of UArtilleryPerActorAbilityMinimum.
 * This is somewhat unusual but allows us vast flexibility in how we lay out abilities.
 * 
 * You'll note there's also a gun key here. While these are instanced abilities, the creation and storage of non-attribute
 * state is strictly prohibited. Doing so may break abruptly, and without explanation, in a myriad of ways. It's not supported.
 * 
 * This means that we'll need to be thoughtful about how we use gameplay tags, so we may need to relax this rule.
 * Rollbacks with gameplay tags and longterm maintenance are both pretty nightmarish, though, so I'd need a solid arg.
 * 
 * Otoh, looman likes them, and that's often a good sign. Technically, we could create a delta-tracking tag container.
 * It's not the worst idea.
 *
 * Like in the UGameplayAbility class it derives from...
 *	// ----------------------------------------------------------------------------------------------------------------
	//
	//	The important functions:
	//	
	//		CanActivateAbility()	- const function to see if ability is activatable. Callable by UI etc
	//
	//		TryActivateAbility()	- Attempts to activate the ability. Calls CanActivateAbility(). Input events can call this directly.
	//								- Also handles instancing-per-execution logic and replication/prediction calls.
	//		
	//		CallActivateAbility()	- Protected, non virtual function. Does some boilerplate 'pre activate' stuff, then calls ActivateAbility()
	//
	//		ActivateAbility()		- What the abilities *does*. This is what child classes want to override.
	//	
	//		CommitAbility()			- Commits reources/cooldowns etc. ActivateAbility() must call this!
	//		
	//		CancelAbility()			- Interrupts the ability (from an outside source).
	//
	//		EndAbility()			- The ability has ended. This is intended to be called by the ability to end itself.
	//	
	// ----------------------------------------------------------------------------------------------------------------

	The K2 functions are wrappers that expose native behavior to the BP graph, and serve a few other functions.
	Check out https://medium.com/trunk-of-code/how-to-easily-change-default-events-to-fit-your-needs-38e87cec16f0
*/

enum FArtilleryStates
{
	Fired, Canceled, CanceledAfterCommit
};



DECLARE_DELEGATE_FourParams(FArtilleryAbilityStateAlert, FArtilleryStates, int, const FGameplayAbilityActorInfo*, const FGameplayAbilityActivationInfo);


UCLASS(BlueprintType)
class ARTILLERYRUNTIME_API UArtilleryPerActorAbilityMinimum : public UGameplayAbility
{
	
	GENERATED_BODY()

	friend struct FArtilleryGun;
	
public:
	//As you can see, they all call through to commit ability.

	FArtilleryAbilityStateAlert GunBinder;
	//ALMOST EVERYTHING THAT IS INTERESTING HAPPENS HERE RIGHT NOW.
	//ONLY ATTRIBUTES ARE REPLICATED. _AGAIN_. ONLY ATTRIBUTES ARE REPLICATED.
	UArtilleryPerActorAbilityMinimum(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer), MyGunKey()
	{
		NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
		ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
		InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Latency Hiding")
	int AvailableDallyFrames = 0;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun")
	FGunKey MyGunKey;
	/*
	// Add the Ability's tags to the given GameplayEffectSpec. This is likely to be overridden per project.
	virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const;
		//this will likelybe where we set up how all artillery abilities behave regarding this.
	*/



	/*/*
	
	The following should likely be overridden differently for cosmetic and non-cosmetic ability base-classes.
	//Invoke a gameplay cue on the ability owner 
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (GameplayTagFilter = "GameplayCue"), DisplayName = "Execute GameplayCue On Owner", meta = (ScriptName = "ExecuteGameplayCue"))
	virtual void K2_ExecuteGameplayCue(FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context);

	//Invoke a gameplay cue on the ability owner, with extra parameters 
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (GameplayTagFilter = "GameplayCue"), DisplayName = "Execute GameplayCueWithParams On Owner", meta = (ScriptName = "ExecuteGameplayCueWithParams"))
	virtual void K2_ExecuteGameplayCueWithParams(FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	//Adds a persistent gameplay cue to the ability owner. Optionally will remove if ability ends
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (GameplayTagFilter = "GameplayCue"), DisplayName = "Add GameplayCue To Owner", meta = (ScriptName = "AddGameplayCue"))
	virtual void K2_AddGameplayCue(FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context, bool bRemoveOnAbilityEnd = true);

	// Adds a persistent gameplay cue to the ability owner. Optionally will remove if ability ends
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (GameplayTagFilter = "GameplayCue"), DisplayName = "Add GameplayCueWithParams To Owner", meta = (ScriptName = "AddGameplayCueWithParams"))
	virtual void K2_AddGameplayCueWithParams(FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameter, bool bRemoveOnAbilityEnd = true);

	// Removes a persistent gameplay cue from the ability owner 
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (GameplayTagFilter = "GameplayCue"), DisplayName = "Remove GameplayCue From Owner", meta = (ScriptName = "RemoveGameplayCue"))
	virtual void K2_RemoveGameplayCue(FGameplayTag GameplayCueTag);
	
	*/

	//here are a few others we'll likely NEED to override.
	///** Returns the time in seconds remaining on the currently active cooldown. */
	//UFUNCTION(BlueprintCallable, Category = Ability)
	//float GetCooldownTimeRemaining() const;
	//virtual float GetCooldownTimeRemaining(const FGameplayAbilityActorInfo* ActorInfo) const;
	//virtual const FGameplayTagContainer* GetCooldownTags() const;
	//virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;
	//virtual bool IsBlockingOtherAbilities() const;

	/**
	 * THIS IS SUPERSEDED BY ACTIVATEBYARTILLERY.
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//InstancingPolicy is ALWAYS EGameplayAbilityInstancingPolicy::NonInstanced for artillery abilities.
	//Storing state outside of tags and game simulation attributes will not be replicated and will cause bugs during rollback.
	//Only implementation graphs in THIS function are called by artillery. Anything else will be ignored.
	//You MUST call end ability and commit ability as appropriate, or execution will not continue.
	//Prefire should use commit\end vs. cancel to signal if execution should continue, but all abilities can.
	UFUNCTION(BlueprintNativeEvent, Category = Ability, DisplayName = "Artillery Ability Implementation", meta=(ScriptName = "ArtilleryActivation"))
	void K2_ActivateViaArtillery(const FGameplayAbilityActorInfo& ActorInfo, const FGameplayEventData& Event);

	//Default behavior, override to use C++!
	virtual void K2_ActivateViaArtillery_Implementation(const FGameplayAbilityActorInfo& ActorInfo,const FGameplayEventData& Event)
	{
	}

	/** Do boilerplate init stuff and then call ActivateAbility */
	//You get a much better sense of how this flows looking at the 
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData = nullptr) override;

	//HEADS UP. END ABILITY FOR ARTILLERY DOES NOT REPLICATE ANYTHING. IT DOES NOT CLEAR TIMERS OR ASYNC BECAUSE
	//YOU ARE NOT SUPPOSED TO USE THEM IN ARTILLERY. IF YOU DO, THEY WILL FAIL ONCE ROLLBACK IS IMPLEMENTED.
	//MAY GOD HAVE MERCY ON OUR SOULS.
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

	/** Generates a GameplayEffectContextHandle from our owner and an optional TargetData.*/
	FGameplayEffectContextHandle GetContextFromOwner(FGameplayAbilityTargetDataHandle OptionalTargetData) const override;

	/** Returns an effect context, given a specified actor info */
	FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

private:


	//these have no function in the Artillery ability sequence.
	void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override {};
	void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override {};
	/** Called from AbilityTask_WaitConfirmCancel to handle input confirming */
	void OnWaitingForConfirmInputBegin() override {};
	void OnWaitingForConfirmInputEnd() override {};

	/** Ability sequences, and artillery in general, make use of only cosmetic events and cues where possible. */
	void SendGameplayEvent(FGameplayTag EventTag, FGameplayEventData Payload) override {};
	
	//TODO: Make sure this is actually obsolete.
	FOnGameplayAbilityEnded OnGameplayAbilityEnded;

	/** Notification that the ability has ended with data on how it was ended */
	FGameplayAbilityEndedDelegate OnGameplayAbilityEndedWithData;

	/** Notification that the ability is being cancelled.  Called before OnGameplayAbilityEnded. */
	FOnGameplayAbilityCancelled OnGameplayAbilityCancelled;
};
