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



class FActionPattern_InternallyStateless
{
public:
	virtual bool runPattern(
		FActionPatternParams fireWith,
		//USED TO DEFINE HOW TO HIDE LATENCY BY TRIMMING LEAD-IN FRAMES OF AN ARTILLERYGUN
		uint32_t leftTrimFrames,
		//USED TO DEFINE HOW TO SHORTEN ARTILLERYGUNS BY SHORTENING DELAYS, SUCH AS DELAYED EXPLOSIONS, TRAJECTORIES, OR SPAWNS, TO HIDE LATENCY.
		uint32_t rightTrimFrames
	) = 0;
};
typedef FActionPattern_InternallyStateless FActionPattern;
