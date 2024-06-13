// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <unordered_map>

#include "GameplayEffectTypes.h"

#include "GameplayEffect.h"
#include "FGunKey.h"
#include "Abilities/GameplayAbility.h"
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
 */
UCLASS(BlueprintType)
class ARTILLERYRUNTIME_API UArtilleryPerActorAbilityMinimum : public UGameplayAbility
{
	GENERATED_BODY()

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Latency Hiding")
	int AvailableDallyFrames = 0;
	//EGameplayAbilityInstancingPolicy::Type InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun")
	FGunKey MyGunKey;
	/*
	// Add the Ability's tags to the given GameplayEffectSpec. This is likely to be overridden per project.
	virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const;
		//this will likelybe where we set up how all artillery abilities behave regarding this.
	*/


	UArtilleryPerActorAbilityMinimum(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
		//this... feels odd, since we're not... instanced? Does a master object still get created?
		//TODO: check that.
		InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	};

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

	//********************
	//This one's fascinating. Unlike the authors of GAS, we actually have a way to reset and cancel abilities without needing instancing on the abilities themselves. Instead,
	//we can use the conserved abilities to walk back any changes to the guns attributes. This might be something for postfire? Fascinating.
	//********************
	/** Destroys instanced-per-execution abilities. Instance-per-actor abilities should 'reset'. Any active ability state tasks receive the 'OnAbilityStateInterrupted' event. Non instance abilities - what can we do? */
	//virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility);

	//These will be the responsibility of the Prefire Abilities. Postfire may modify cooldowns, to implement things like "refresh if kills enemy"
	//Most of these systems are, frankly, not very useful. In general, tags are not the best way to communicate if an ability should go into cooldown.
	//Honestly, I'm pretty reticient about tags in general.

	///** Returns true if this ability can be activated right now. Has no side effects */
	//virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	///** Returns true if this ability can be triggered right now. Has no side effects */
	//virtual bool ShouldAbilityRespondToEvent(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* Payload) const;

	///** Returns true if an an ability should be activated */
	//virtual bool ShouldActivateAbility(ENetRole Role) const;

	///** Returns the time in seconds remaining on the currently active cooldown. */
	//UFUNCTION(BlueprintCallable, Category = Ability)
	//float GetCooldownTimeRemaining() const;

	///** Returns the time in seconds remaining on the currently active cooldown. */
	//virtual float GetCooldownTimeRemaining(const FGameplayAbilityActorInfo* ActorInfo) const;

	///** Returns the time in seconds remaining on the currently active cooldown and the original duration for this cooldown. */
	//virtual void GetCooldownTimeRemainingAndDuration(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& TimeRemaining, float& CooldownDuration) const;

	///** Returns all tags that can put this ability into cooldown */
	//virtual const FGameplayTagContainer* GetCooldownTags() const;

	///** Returns true if none of the ability's tags are blocked and if it doesn't have a "Blocking" tag and has all "Required" tags. */
	//virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	///** Returns true if this ability is blocking other abilities */
	//virtual bool IsBlockingOtherAbilities() const;

	///** Sets rather ability block flags are enabled or disabled. Only valid on instanced abilities */
	//UFUNCTION(BlueprintCallable, Category = Ability)
	//virtual void SetShouldBlockOtherAbilities(bool bShouldBlockAbilities);
};
