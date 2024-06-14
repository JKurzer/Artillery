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
#include "FArtilleryNoGuaranteeReadOnly.h"
#include <string>

//this is vulnerable to memoization but I can't think of a pretty way to do that which doesn't make rollback insane to debug.
//as a result, these lil fellers are stateless. If you wanna do a memoized version, I recommend it strongly, but make sure profiling
//shows that's actually necessary. I sincerely doubt it will be.

class FActionPattern_InternallyStateless
{
public:
	virtual uint32_t runPattern(uint64_t frameToRunBackFrom, 
		FActionBitMask ToSeekUnion,
		FANG_PTR Buffer
	) = 0;

	virtual const FString getName() = 0;
	static const inline FString Name = "FActionPattern_InternallyStatelessPattern"; //you should never see this as getName is virtual.
};

typedef FActionPattern_InternallyStateless FActionPattern;

//TODO: ALWAYS customize this to the sample-rate you select for cabling. ALWAYS. Or your game will feel Real Bad.
constexpr const inline int HoldSweepBack = 5; // this is literally the sin within the beast. 

class FActionPattern_SingleFrameFire : public FActionPattern_InternallyStateless
{
public:
	virtual uint32_t runPattern(uint64_t frameToRunBackFrom,
		FActionBitMask ToSeekUnion,
		FANG_PTR Buffer
	)
	override
	{
		return Buffer->peek(frameToRunBackFrom)->GetButtonsAndEventsFlat() & ToSeekUnion.getFlat();
		
	};
	const FString getName() override { return Name; };
	static const inline FString Name = "FActionPattern_SingleFrameFirePattern";
};


class FActionPattern_ButtonHold : public FActionPattern_InternallyStateless
{
public:
	// returned pattern will tell us which inputs (button/events) were held
	virtual uint32_t runPattern(uint64_t frameToRunBackFrom,
		FActionBitMask ToSeekUnion,
		FANG_PTR Buffer
	)
		override
	{
		/*
		  to allow for one missed input in each input bit:
		  tracker = mask
		   for x in input
			outcome = mask & x
			mask = tracker | outcome
			tracker &= outcome*/

		uint64_t startIndex = (frameToRunBackFrom - HoldSweepBack < 0) ? 0 : frameToRunBackFrom - HoldSweepBack;
		uint32_t toSeek = ToSeekUnion.getFlat();
		uint32_t tracker = toSeek;
		uint32_t outcome = 0;
		uint32_t x = 0;

		// std::optional<FArtilleryShell>(CurrentHistory[input])
		for (uint64_t i = startIndex; i <= frameToRunBackFrom; ++i)
		{
			//I am ??
			x = (Buffer[i]).get();
			outcome = toSeek & x;
			toSeek = tracker | outcome;
			tracker &= outcome;
		}
		// allows for 1 missed input for each input across the time frame, but still count as hold
		// this implementation does not track where in the sequence the drops were
		return outcome;
	};
	const FString getName() override { return Name; };
	static const inline FString Name = "FActionPattern_ButtonHoldPattern";
};

//TODO: make sure the right runPattern gets called. can't rem the inheritance rules and I don't have time to rabbit hole.
class FActionPattern_ButtonReleaseNoDelay : public FActionPattern_ButtonHold 
{
public:
	virtual uint32_t runPattern(uint64_t frameToRunBackFrom,
		FActionBitMask ToSeekUnion,
		FANG_PTR Buffer
	)
		override
	{
		//super::runpattern with frametorunbackfrom - 1
		//using invert of ToSeekUnion
		//runpattern frametorunbackfrom
		//& results.
		return false; //return results
	};
	const FString getName() override { return Name; };
	static const inline FString Name = "FActionPattern_ButtonReleaseNoDelay";
};

//TODO: make sure the right runPattern gets called. can't rem the inheritance rules and I don't have time to rabbit hole.
class FActionPattern_ButtonReleaseOneFrameDelay : public FActionPattern_ButtonReleaseNoDelay
{
public:
	virtual uint32_t runPattern(uint64_t frameToRunBackFrom,
		FActionBitMask ToSeekUnion,
		FANG_PTR Buffer
	)
		override
	{	//using toseekunion (super inverts)
		//super::runpattern frametorunbackfrom - 1
		//inverting toseekunion
		//runpattern frametorunbackfrom
		//& results.
		return false; //return results
	};
	const FString getName() override { return Name; };
	static const inline FString Name = "FActionPattern_ButtonReleaseOneFrameDelay";
};

class FActionPattern_StickFlick : public FActionPattern_InternallyStateless
{
public:
	virtual uint32_t runPattern(uint64_t frameToRunBackFrom,
		FActionBitMask ToSeekUnion,
		FANG_PTR Buffer
	)
		override
	{
		//super::runpattern with frametorunbackfrom - 1
		//using invert of ToSeekUnion
		//runpattern frametorunbackfrom
		//& results.
		return false; //return results
	};
	const FString getName() override { return Name; };
	static const inline FString Name = "FActionPattern_StickFlick";
};