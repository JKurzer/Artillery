// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"

#include "ArtilleryCommonTypes.h"
#include "ArtilleryDispatch.h"
#include "FAttributeMap.generated.h"

USTRUCT(BlueprintType)
struct ARTILLERYRUNTIME_API FAttributeMap
{
	GENERATED_BODY()
	
	AttrMapPtr MyAttributes;
	ActorKey ParentKey;
	UArtilleryDispatch* MyDispatch = nullptr;
	bool ReadyToUse = false;

	// Don't use this default constructor, this is a bad
	FAttributeMap()
	{
		
	};

	FAttributeMap(ActorKey ParentKeyIn, UArtilleryDispatch* MyDispatchIn, TMap<AttribKey, double> DefaultAttributesIn)
	{
		Initialize(ParentKeyIn, MyDispatchIn, DefaultAttributesIn);
	};

	void Initialize(ActorKey ParentKeyIn, UArtilleryDispatch* MyDispatchIn, TMap<AttribKey, double> DefaultAttributesIn)
	{
		this->ParentKey = ParentKeyIn;
		this->MyDispatch = MyDispatchIn;

		this->MyAttributes = MakeShareable(new AttributeMap());
		
		//TODO: swap this to loading values from a data table, and REMOVE this fallback.
		//If we want defaults, those defaults should ALSO live in a data table, that way when a defaulting bug screws us
		//maybe we can fix it without going through a full cert using a data only update.
		for(auto x : DefaultAttributesIn)
		{
			MyAttributes->Add(x.Key, MakeShareable(new FConservedAttributeData));
			MyAttributes->FindChecked(x.Key)->SetBaseValue(x.Value);
			MyAttributes->FindChecked(x.Key)->SetCurrentValue(x.Value);
		}

		MyDispatch->RegisterAttributes(ParentKey, MyAttributes);

		ReadyToUse = true;
	};
	
	~FAttributeMap()
	{
		MyDispatch->DeregisterAttributes(ParentKey);
	}
};