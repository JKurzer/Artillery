// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Templates/SubclassOf.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "AttributeSet.h"
#include <bitset>
#include "Containers/CircularBuffer.h"
#include "FGunKey.generated.h"

USTRUCT(BlueprintType)
struct FGunKey
{
	GENERATED_BODY()
public:
	//TODO: this needs to be removed. we should never allow a default gunkey.
	FGunKey()
	{}
	FGunKey(FString Name, uint64_t id): 
	GunDefinitionID(Name), GunInstanceID(id)
	{
	}

	FString GunDefinitionID; //this will need to be human searchable
	uint64_t GunInstanceID;

	bool operator == (const struct FGunKey& Other)
	{
		return GunInstanceID == Other.GunInstanceID;
	};
	friend uint32 GetTypeHash(const FGunKey& Other)
	{
		// it's probably fine!
		return GetTypeHash(Other.GunDefinitionID) + GetTypeHash(Other.GunInstanceID);
	}
};
bool operator==(FGunKey const& lhs, FGunKey const& rhs) {
	return (lhs.GunDefinitionID == rhs.GunDefinitionID) && (lhs.GunInstanceID == rhs.GunInstanceID);
}
