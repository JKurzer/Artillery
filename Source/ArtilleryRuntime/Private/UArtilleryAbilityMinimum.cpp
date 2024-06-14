#include "UArtilleryAbilityMinimum.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/Engine.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayCue_Types.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Misc/DataValidation.h"
#include "UObject/Package.h"
#include "UFireControlMachine.h"

//The K2 function paths for ending abilities are a little confusing, so I've excerpted the original code below.
//We don't override it atm, so this is how you should currently expect Artillery abilities to behave just like UGAs.
//Here's the call path that UGameplayAbility exposes to blueprint which calls end, commit, and cancel.
/*
bool UGameplayAbility::K2_CommitAbility()
{
	check(CurrentActorInfo);
	return CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
}

bool UGameplayAbility::K2_CommitAbilityCooldown(bool BroadcastCommitEvent, bool ForceCooldown)
{
	check(CurrentActorInfo);
	if (BroadcastCommitEvent)
	{
		UAbilitySystemComponent* const AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo_Checked();
		AbilitySystemComponent->NotifyAbilityCommit(this);
	}
	return CommitAbilityCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, ForceCooldown);
}

bool UGameplayAbility::K2_CommitAbilityCost(bool BroadcastCommitEvent)
{
	check(CurrentActorInfo);
	if (BroadcastCommitEvent)
	{
		UAbilitySystemComponent* const AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo_Checked();
		AbilitySystemComponent->NotifyAbilityCommit(this);
	}
	return CommitAbilityCost(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
}*/

//As you can see, they all call through to commit ability.

void UArtilleryPerActorAbilityMinimum::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

	if (TriggerEventData && bHasBlueprintActivateFromEvent)
	{
		// A Blueprinted ActivateAbility function must call CommitAbility somewhere in its execution chain.
		K2_ActivateAbilityFromEvent(*TriggerEventData);
	}
	else if (bHasBlueprintActivate)
	{
		// A Blueprinted ActivateAbility function must call CommitAbility somewhere in its execution chain.
		K2_ActivateAbility();
	}
	else if (bHasBlueprintActivateFromEvent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Artillery Ability %s expects event data but none is being supplied. Use 'Activate Ability' instead of 'Activate Ability From Event' in the Blueprint."), *GetName());
		constexpr bool bReplicateEndAbility = false;
		constexpr bool bWasCancelled = true;
		EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Artillery Ability %s expects to be activated by FArtilleryGun with a valid Blueprint Graph implemented and an event."), *GetName());
		constexpr bool bReplicateEndAbility = false;
		constexpr bool bWasCancelled = true;
		EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
		// Per the original code from UGA, we'll need to expand this significantly.
		
		//	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		//	{			
		//		constexpr bool bReplicateEndAbility = true;
		//		constexpr bool bWasCancelled = true;
		//		EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
		//	}
	}
}

void UArtilleryPerActorAbilityMinimum::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
		//right now, we do EXACTLY AND ONLY what UGA does. This is going to get less and less true.
		Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);
}

void UArtilleryPerActorAbilityMinimum::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			UE_LOG(LogTemp, Verbose, TEXT("Attempting to end Ability %s but ScopeLockCount was greater than 0, adding end to the WaitingToExecute Array"), *GetName());
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &UArtilleryPerActorAbilityMinimum::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
			return;
		}
        
        if (GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
        {
            bIsAbilityEnding = true;
        }

		// Give blueprint a chance to react
		K2_OnEndAbility(bWasCancelled);

		// Protect against blueprint causing us to EndAbility already
		if (bIsActive == false && GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			return;
		}



		if (GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			bIsActive = false;
			bIsAbilityEnding = false;
		}

		// Tell all our tasks that we are finished and they should cleanup
		for (int32 TaskIdx = ActiveTasks.Num() - 1; TaskIdx >= 0 && ActiveTasks.Num() > 0; --TaskIdx)
		{
			UGameplayTask* Task = ActiveTasks[TaskIdx];
			if (Task)
			{
				Task->TaskOwnerEnded();
			}
		}
		ActiveTasks.Reset();	// Empty the array but don't resize memory, since this object is probably going to be destroyed very soon anyways.

		if (UAbilitySystemComponent* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
		{
			// Remove tags we added to owner
			AbilitySystemComponent->RemoveLooseGameplayTags(ActivationOwnedTags);
			
			// Remove tracked GameplayCues that we added
			for (FGameplayTag& GameplayCueTag : TrackedGameplayCues)
			{
				AbilitySystemComponent->RemoveGameplayCue(GameplayCueTag);
			}
			TrackedGameplayCues.Empty();

			if (CanBeCanceled())
			{
				// If we're still cancelable, cancel it now
				AbilitySystemComponent->HandleChangeAbilityCanBeCanceled(AbilityTags, this, false);
			}
			
			if (IsBlockingOtherAbilities())
			{
				// If we're still blocking other abilities, cancel now
				AbilitySystemComponent->ApplyAbilityBlockAndCancelTags(AbilityTags, this, false, BlockAbilitiesWithTag, false, CancelAbilitiesWithTag);
			}

			AbilitySystemComponent->ClearAbilityReplicatedDataCache(Handle, CurrentActivationInfo);

			// Tell owning AbilitySystemComponent that we ended so it can do stuff (including MarkPendingKill us)
			AbilitySystemComponent->NotifyAbilityEnded(Handle, this, bWasCancelled);
		}

		if (IsInstantiated())
		{
			CurrentEventData = FGameplayEventData{};
		}

		// WE DO NOT UNBIND as our single instance per actor is held by an FArtillery gun under the control of
		// both the ability system component and the Artillery fire control.
	
		FArtilleryStates HowDidItGo = bWasCancelled ? FArtilleryStates::Fired : FArtilleryStates::Canceled ;
		GunBinder.Execute(HowDidItGo, AvailableDallyFrames, ActorInfo, ActivationInfo);
	}
}

bool UArtilleryPerActorAbilityMinimum::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags)
{
	
	return Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
}


FGameplayEffectContextHandle UArtilleryPerActorAbilityMinimum::GetContextFromOwner(FGameplayAbilityTargetDataHandle OptionalTargetData) const
{
	return Super::GetContextFromOwner(OptionalTargetData);
}

FGameplayEffectContextHandle UArtilleryPerActorAbilityMinimum::MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	return Super::MakeEffectContext(Handle, ActorInfo);
}

void UArtilleryPerActorAbilityMinimum::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}
