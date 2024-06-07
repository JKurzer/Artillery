// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <unordered_map>

#include "GameplayEffectTypes.h"

#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"
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
 * Try
 * Prefire
 * WindUp
 * Activate
 * WindDown
 * PostFire
 * Commit
 * End
 * 
 * Either Commit OR end may be called.
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
class ARTILLERYRUNTIME_API UArtilleryAbilitySequence : public UArtilleryUninstancedAbilityMinimum
{
	GENERATED_BODY()

public:

};
