// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "FMasks.h"
#include "AttributeSet.h"
#include <bitset>
#include "Containers/CircularBuffer.h"
#include "FActionBitMask.generated.h"


//we INTENTIONALLY lose our connection to the bristlecone
//type system here.


USTRUCT(BlueprintType)
 struct FActionBitMask
{
	GENERATED_BODY()
public:
	
	std::bitset<Arty::Intents::TYPEBREAK_MAPPING_FROM_BC_BUTTONS> buttons;
	std::bitset<Arty::Intents::TYPEBREAK_MAPPING_FROM_BC_EVENTS> events;

	
	uint32_t getFlat()
	{
		uint32_t result = buttons.to_ulong() << Arty::Intents::TYPEBREAK_MAPPING_FROM_BC_EVENTS;
		return (result | events.to_ulong());
	};
	friend uint32 GetTypeHash(const FActionBitMask& Other)
	{
		// it's probably fine!
		uint32_t result = Other.buttons.to_ulong() << Arty::Intents::TYPEBREAK_MAPPING_FROM_BC_EVENTS;
		return GetTypeHash((result | Other.events.to_ulong()));//it is very tempting to not hash, but that will break distribs and hotspot.
	};
};

