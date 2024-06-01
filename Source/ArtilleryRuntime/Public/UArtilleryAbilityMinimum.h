// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <unordered_map>
#include "Abilities/GameplayAbility.h"
#include "UArtilleryAbilityMinimum.generated.h"

/**
 * This is more of a note with the idea embodied in code. Still not sure I love it, but...
 * I think the simplest way to fix a lot of our problems is to add two concepts to GAS:
 * ActivateLate and AtMostOnce GameplayCues to make sure things don't get re-fired.
 * 
 * 
 * 
 * AtMostOnce:
 * To ensure gameplay cues only activate once, I think we can do just some pretty simple trickery.
 * All Abilities in artillery are required to parent from this class.
 * 
 */
UCLASS()
class ARTILLERYRUNTIME_API UArtilleryInstancedAbilityMinimum : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Latency Hiding")
	bool RerunDueToReconcile = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Latency Hiding")
	int AvailableDallyFrames = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Latency Hiding")
	int DallyFramesToOmit = 0;

};
