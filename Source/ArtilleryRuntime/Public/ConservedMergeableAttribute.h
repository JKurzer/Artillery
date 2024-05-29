// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "AttributeSet.h"
#include "Containers/CircularBuffer.h"
//#include "ConservedMergeableAttribute.generated.h"
/**
 * Conserved mergeable attributes record their values in a buffer when changed during runtime
 * and to disk when changed in editor outside of PIE. This may actually require an overload of
 * FGameplayAttribute, not just FGameplayAttributeData. At the moment, this is a placeholder file
 * cause there's a couple details I haven't figured out. The Attribute Initter actually provides
 * a really clean example of how this should work, but I want it to be able to write back.
 * 
 * I haven't decided if I want to use configs, CSVs, or the design I've been contemplating for the blueprint
 * problem. It's a surprisingly difficult question. Regardless, the goal of this is to allow transparently
 * mergeable attributes, and serve as a template for extending this through-out artillery.
 */
/*
USTRUCT(BlueprintType)
struct ARTILLERYRUNTIME_API FConservedAttributeData : public FGameplayAttributeData
{
	GENERATED_BODY()
	uint64_t counterBase = 0;
	uint64_t counterCurrent = 0;
	TCircularBuffer<float> CurrentHistory = TCircularBuffer<float>(128);
	TCircularBuffer<float> BaseHistory = TCircularBuffer<float>(128);

	virtual void SetCurrentValue(float NewValue) override {
		CurrentHistory[CurrentHistory.GetNextIndex(counterBase)] = CurrentValue;
		CurrentValue = NewValue;
		++counterBase;
	};

	virtual void SetBaseValue(float NewValue) override {
		BaseHistory[BaseHistory.GetNextIndex(counterBase)] = BaseValue;
		BaseValue = NewValue;
		++counterBase;
	};

};
*/