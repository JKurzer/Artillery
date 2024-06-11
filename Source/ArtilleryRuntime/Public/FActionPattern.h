// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "AttributeSet.h"

#include "ArtilleryCommonTypes.h"
#include <bitset>
#include "Containers/CircularBuffer.h"
#include <string>



class FActionPattern_InternallyStateless
{
public:
	virtual bool runPattern(
		FActionPatternParams fireWith
	) = 0;

	virtual const std::string_view getName() = 0;
	static constexpr std::string_view Name = "InternallyStatelessPattern"; //you should never see this as getName is virtual.
};

typedef FActionPattern_InternallyStateless FActionPattern;

class FActionPattern_SingleFrameFire : public FActionPattern_InternallyStateless
{
public:
	bool runPattern (
		FActionPatternParams fireWith
	) 
	override
	{

		return false;
	};
	const std::string_view getName() override { return Name; };
	static constexpr std::string_view Name = "SingleFrameFirePattern";
};
