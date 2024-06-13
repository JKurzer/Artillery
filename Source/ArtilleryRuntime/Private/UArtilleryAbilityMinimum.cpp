#include "UArtilleryAbilityMinimum.h"

void UArtilleryPerActorAbilityMinimum::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
}

void UArtilleryPerActorAbilityMinimum::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
}

void UArtilleryPerActorAbilityMinimum::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
}

bool UArtilleryPerActorAbilityMinimum::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags)
{
	return false;
}



FGameplayEffectContextHandle UArtilleryPerActorAbilityMinimum::GetContextFromOwner(FGameplayAbilityTargetDataHandle OptionalTargetData) const
{
	return FGameplayEffectContextHandle();
}

FGameplayEffectContextHandle UArtilleryPerActorAbilityMinimum::MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	return FGameplayEffectContextHandle();
}
