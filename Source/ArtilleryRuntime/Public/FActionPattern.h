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
constexpr const inline int HoldDropAllowance = 1; // number of missed inputs allowed in a hold sequence

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
	virtual uint32_t runPattern(uint64_t frameToRunBackFrom,
		FActionBitMask ToSeekUnion,
		FANG_PTR Buffer
	)
		override
	{
		int heldInputs = 0;
		for (int i = HoldSweepBack; i >= 0; --i)
		{
			if (Buffer->peek(frameToRunBackFrom - i)->GetButtonsAndEventsFlat() & ToSeekUnion.getFlat())
			{
				heldInputs++;
			}
		}
		// allows for a small fraction of missed inputs (# is HoldDropAllowance), but still count as hold
		// this implementation does not track where in the sequence the drops were
		return heldInputs >= (HoldSweepBack - HoldDropAllowance);
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