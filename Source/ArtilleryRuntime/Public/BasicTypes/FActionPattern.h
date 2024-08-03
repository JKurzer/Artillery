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

enum ActionPatternKey
{
	InternallyStateless = 0,
	SingleFrameFire = 1,
	ButtonHoldAllowOneMiss = 2,
	ButtonHold = 3,
	ButtonReleaseNoDelay = 4,
	StickFlick = 5,
	OnPress = 6
};
typedef ActionPatternKey ArtIPMKey;//artillery intent pattern matcher key

class FActionPattern_InternallyStateless
{
public:
	virtual uint32_t const runPattern(uint64_t frameToRunBackFrom, 
		FActionBitMask& ToSeekUnion,
		FANG_PTR Buffer
	) const = 0;

	virtual ArtIPMKey const getName() const = 0;
	static const inline ArtIPMKey Name = ArtIPMKey::InternallyStateless; //you should never see this as getName is virtual.
};

typedef FActionPattern_InternallyStateless FActionPattern;



class FActionPattern_SingleFrameFire : public FActionPattern_InternallyStateless
{
public:
	virtual uint32_t const runPattern(uint64_t frameToRunBackFrom,
		FActionBitMask& ToSeekUnion,
		FANG_PTR Buffer
	)
	const override
	{
		return Buffer->peek(frameToRunBackFrom)->GetButtonsAndEventsFlat() & ToSeekUnion.getFlat();
		
	};
	const ArtIPMKey getName() const override { return Name; };
	static const inline ArtIPMKey Name = ArtIPMKey::SingleFrameFire;
};

class FActionPattern_ButtonHoldAllowOneMiss : public FActionPattern_InternallyStateless
{
public:
	// returned pattern will tell us which inputs (button/events) were held
	virtual uint32_t const runPattern(uint64_t frameToRunBackFrom,
		FActionBitMask& ToSeekUnion,
		FANG_PTR Buffer
	)
	const override
	{
		/*
		  to allow for one missed input in each input bit:
		  tracker = mask
		   for x in input
			outcome = mask & x
			mask = tracker | outcome
			tracker &= outcome*/

		uint64_t StartIndex =
			(frameToRunBackFrom - ArtilleryHoldSweepBack < 0) ?
				0 : frameToRunBackFrom - ArtilleryHoldSweepBack;
		uint32_t toSeek = ToSeekUnion.getFlat();
		uint32_t tracker = toSeek;
		uint32_t outcome = 0;
		uint32_t x = 0;

		// std::optional<FArtilleryShell>(CurrentHistory[input])
		for (uint64_t i = StartIndex; i <= frameToRunBackFrom; ++i)
		{
			x = Buffer->peek(i)->GetButtonsAndEventsFlat();
			outcome = toSeek & x;
			toSeek = tracker | outcome;
			tracker &= outcome;
		}
		// allows for 1 missed input for each input across the time frame, but still count as hold
		// this implementation does not track where in the sequence the drops were
		return outcome;
	};
	const ArtIPMKey getName() const override { return Name; };
	static const inline ArtIPMKey Name = ArtIPMKey::ButtonHoldAllowOneMiss;
};

class FActionPattern_OnPress : public FActionPattern_InternallyStateless
{
public:
	// returned pattern will tell us which inputs (button/events) were held
	virtual uint32_t const runPattern(uint64_t frameToRunBackFrom,
		FActionBitMask& ToSeekUnion,
		FANG_PTR Buffer
	)
		const override
	{
		uint64_t StartIndex =
			(frameToRunBackFrom - ArtilleryHoldSweepBack < 0) ?
				0 : frameToRunBackFrom - ArtilleryHoldSweepBack;
		uint32_t toSeek = ToSeekUnion.getFlat();

		//do NOT check current frame (< instead of <=)
		for (uint64_t i = StartIndex; i < frameToRunBackFrom; ++i)
		{
			toSeek = ((Buffer->peek(i)->GetButtonsAndEventsFlat()) ^ toSeek) & toSeek;
		}
		
		// this implementation does not track where in the sequence the drops were
		return toSeek & (Buffer->peek(frameToRunBackFrom)->GetButtonsAndEventsFlat() & ToSeekUnion.getFlat());
	};
	const ArtIPMKey getName() const override { return Name; };
	static const inline ArtIPMKey Name = ArtIPMKey::OnPress;
};


class FActionPattern_ButtonHold : public FActionPattern_InternallyStateless
{
public:
	// returned pattern will tell us which inputs (button/events) were held
	virtual uint32_t const runPattern(uint64_t frameToRunBackFrom,
		FActionBitMask& ToSeekUnion,
		FANG_PTR Buffer
	)
		const override
	{
		/*
		  to allow for one missed input in each input bit:
		  tracker = mask
		   for x in input
			outcome = mask & x
			mask = tracker | outcome
			tracker &= outcome*/

		uint64_t StartIndex =
			(frameToRunBackFrom - ArtilleryHoldSweepBack < 0) ?
				0 : frameToRunBackFrom - ArtilleryHoldSweepBack;
		uint32_t toSeek = ToSeekUnion.getFlat();
		uint32_t x = 0;

		// std::optional<FArtilleryShell>(CurrentHistory[input])
		for (uint64_t i = StartIndex; i <= frameToRunBackFrom; ++i)
		{
			x = Buffer->peek(i)->GetButtonsAndEventsFlat();
			toSeek = toSeek & x;
		}
		
		// this implementation does not track where in the sequence the drops were
		return toSeek;
	};
	const ArtIPMKey getName() const override { return Name; };
	static const inline ArtIPMKey Name = ArtIPMKey::ButtonHold;
};

class FActionPattern_ButtonReleaseNoDelay : public FActionPattern_ButtonHold 
{
public:
	virtual uint32_t const runPattern(uint64_t frameToRunBackFrom,
		FActionBitMask& ToSeekUnion,
		FANG_PTR Buffer
	)
		const override
	{
		// held before this frame?
		uint32_t heldBefore = FActionPattern_ButtonHold::runPattern(frameToRunBackFrom - 1, ToSeekUnion, Buffer);
		// not held this frame?
		uint32_t releasedNow = Buffer->peek(frameToRunBackFrom)->GetButtonsAndEventsFlat() & ~ToSeekUnion.getFlat();
		// release is held -> not held
		return heldBefore & releasedNow;
	};
	const ArtIPMKey getName() const override { return Name; };
	static const inline ArtIPMKey Name = ArtIPMKey::ButtonReleaseNoDelay;
};


//NOTE: if you want to check if buttons were held across the whole stick-flick
//you will need to do that separately or create a new pattern. The flick is ALREADY
//expensive.
//NOTE THIS USES THE FLICK SWEEPBACK which is INCLUSIVE
class FActionPattern_StickFlick : public FActionPattern_InternallyStateless
{
public:
	virtual uint32_t const runPattern(uint64_t frameToRunBackFrom,
		FActionBitMask& ToSeekUnion,
		FANG_PTR Buffer
	)
		const override
	{
		//NOTE THIS USES THE FLICK SWEEPBACK which is INCLUSIVE
		auto cur = Buffer->peek(frameToRunBackFrom);
		//for a VARIETY OF REASONS we really don't want to start detecting flicks early.
		if(frameToRunBackFrom - ArtilleryFlickSweepBack < ArtilleryFlickSweepBack)
		{
			return 0;
		}
		uint64_t FinishIndex = frameToRunBackFrom - ArtilleryFlickSweepBack;
		//we never turn them into floats here.
		uint32_t curX = cur->GetStickLeftXAsACSN(); 
		uint32_t curY = cur->GetStickLeftYAsACSN();
		uint32_t sqrMagnitude = (curX * curX) + (curY * curY);
		//et viola.
		if (sqrMagnitude > ArtilleryMagicFlickBoundary) {
			//AND it sweeps backward, not forward.
			Buffer->peek(frameToRunBackFrom);
			for (uint64_t index = frameToRunBackFrom-1; FinishIndex <= index; --index)
				{
				auto entry = Buffer->peek(index);
				auto x =  entry->GetStickLeftXAsACSN() - curX;
				auto y=entry->GetStickLeftYAsACSN()- curY;
				if (((x*x) + (y*y)  ) >= ArtilleryMagicMinimumFlickDistanceRequired) {
					return ToSeekUnion.getFlat();
				}
			}
		}
		return 0; //return results
	};
	const ArtIPMKey getName() const override { return Name; };
	static const inline ArtIPMKey Name = ArtIPMKey::StickFlick;
};

namespace Arty
{
	namespace IPM
	{

		typedef const FActionPattern* CanonPattern;
		constexpr const FActionPattern_StickFlick Flick;
		constexpr const CanonPattern GFlick = &Flick;
		constexpr const FActionPattern_ButtonReleaseNoDelay Release;
		constexpr const CanonPattern GRelease = &Release;
		constexpr const FActionPattern_SingleFrameFire FramePress;
		constexpr const CanonPattern GPress = &FramePress;
		constexpr const FActionPattern_ButtonHold Hold;
		constexpr const CanonPattern GHold = &Hold;
		constexpr const FActionPattern_ButtonHoldAllowOneMiss HoldWMiss;
		constexpr const CanonPattern GHoldWM = &HoldWMiss;
		constexpr const FActionPattern_OnPress StartOfPress;
		constexpr const CanonPattern GPerPress = &StartOfPress;
	}
}
