// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "AttributeSet.h"
#include "Containers/CircularBuffer.h"
#include "ConservedAttribute.generated.h"
/**
 * Conserved attributes record their last 128 changes.
 * Currently, this is for debug purposes, but we can use it with some additional features to provide a really expressive
 * model for rollback at a SUPER granular level if needed. 
 */


USTRUCT(BlueprintType)
struct ARTILLERYRUNTIME_API FConservedAttributeData : public FGameplayAttributeData
{
	GENERATED_BODY()
	uint64_t counterBase = 0;
	uint64_t counterCurrent = 0;
	TCircularBuffer<double> CurrentHistory = TCircularBuffer<double>(128);
	TCircularBuffer<double> RemoteHistory = TCircularBuffer<double>(128);
	TCircularBuffer<double> BaseHistory = TCircularBuffer<double>(128);

	virtual void SetCurrentValue(float NewValue) override {
		SetCurrentValue(static_cast<double>(NewValue));
	};

	virtual void SetCurrentValue(double NewValue) {
		CurrentHistory[CurrentHistory.GetNextIndex(counterBase)] = CurrentValue;
		CurrentValue = NewValue;
		++counterBase;
	};

	virtual void SetRemoteValue(float NewValue) {
		SetRemoteValue(static_cast<double>(NewValue));
	};
	
	virtual void SetRemoteValue(double NewValue) {
		RemoteHistory[RemoteHistory.GetNextIndex(counterBase)] = NewValue;
		++counterBase;
	};
	
	virtual void SetBaseValue(float NewValue) override {
		SetBaseValue(static_cast<double>(NewValue));
	};

	virtual void SetBaseValue(double NewValue) {
		BaseHistory[BaseHistory.GetNextIndex(counterBase)] = BaseValue;
		BaseValue = NewValue;
		++counterBase;
	};
	
};
