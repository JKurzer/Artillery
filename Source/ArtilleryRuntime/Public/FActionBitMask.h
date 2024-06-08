// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "AttributeSet.h"
#include <bitset>
#include "Containers/CircularBuffer.h"
#include "FActionBitMask.generated.h"

USTRUCT(BlueprintType)
struct FActionBitMask
{
	GENERATED_BODY()
public:
	std::bitset<14> buttons;
	std::bitset<6> events;
};

