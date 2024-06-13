#include "UArtilleryAbilitySequence.h"

void UArtilleryAbilitySequence::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
}

void UArtilleryAbilitySequence::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
}

void UArtilleryAbilitySequence::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
}

bool UArtilleryAbilitySequence::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags)
{
	return false;
}



FGameplayEffectContextHandle UArtilleryAbilitySequence::GetContextFromOwner(FGameplayAbilityTargetDataHandle OptionalTargetData) const
{
	return FGameplayEffectContextHandle();
}

FGameplayEffectContextHandle UArtilleryAbilitySequence::MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	return FGameplayEffectContextHandle();
}
