// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "UArtilleryAbilityMinimum.generated.h"

/**
 * This is more of a note with the idea embodied in code. Still not sure I love it, but...
 * I think the simplest way to fix a lot of our problems is to add two concepts to GAS:
 * ActivateLate and AtMostOnce GameplayCues to make sure things don't get re-fired.
 * 
 * ActivateLate comes in three forms:
 * ActivateOneLate
 * ActivateTwoLate
 * ActivateInstantly
 * 
 * To ensure gameplay cues only activate once, I think we can do just some pretty simple trickery.
 * All Abilities in artillery are required to parent from this class.
 * 
 * I want to make sure that OneLate and TwoLate implementation isn't a burden. That'll need
 * some serious thought put into it. What's really nice, though, is that this lets you compose
 * abilities in some REALLY cool ways, ensuring that most abilities can see really heavy reuse.
 * 
 * That's got downsides, too. Gonna have to be mindful. Anyway, that's what this would do.
 * 
 * 
 */
UCLASS()
class ARTILLERYRUNTIME_API UArtilleryAbilityMinimum : public UGameplayAbility
{
	GENERATED_BODY()
	/*
	virtual ActivateOneLate
	virtual ActivateTwoLate
	virtual ActivateInstantly
		would all be pure virtual.	
	*/

};
