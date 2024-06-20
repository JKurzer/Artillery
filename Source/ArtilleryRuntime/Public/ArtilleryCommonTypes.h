// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
/**
 * Component for managing input streams in an ECS-like way, where any controller can request any stream.
 */


#include "FActionBitMask.h"
#include "BristleconeCommonTypes.h"

namespace Arty
{
	typedef TheCone::PacketElement INNNNCOMING;
	typedef uint32_t InputStreamKey;
	typedef uint32_t PlayerKey;
	typedef uint32_t ActorKey;
	typedef uint32_t FireControlKey;
	using BristleTime = long; //this will become uint32. don't bitbash this.
	using ArtilleryTime = BristleTime;
}






#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "AttributeSet.h"
#include <bitset>
#include "Containers/CircularBuffer.h"
#include "FGunKey.h"

//PATH TO DATA TABLES
//constexpr const FString GunsManifests = "";
//TODO: ALWAYS customize this to the sample-rate you select for cabling. ALWAYS. Or your game will feel Real Bad.
constexpr int ArtilleryInputSearchWindow = 70;
constexpr const inline int ArtilleryHoldSweepBack = 5; // this is literally the sin within the beast. 
constexpr const inline int ArtilleryFlickSweepBack = 15; // And this is no better.

//But this is pretty good. Here it is, the most magical constant I've written.
//we use the Cabling Integerized Sticks. Normally, we turn them into floats.
//But squares of floats are a good way to blow your bit corruption limits for
//FP rounding error. So here, we actually use the sqrMag of the debiased ints.
//It works just the same. It's distance across a metric space, number of discretized
//positions away from center on an axis. So we need a magnitude boundary to start
//a stick flick detection from. This is that.
//This is 2*(975*975)
constexpr const inline uint32_t ArtilleryMagicFlickBoundary = 1901250;
//these should LIKELY be the same, but I could see an argument than this might need to be a little smaller?
//Tune as needed. we actually maintain a surprisingly finegrained degree of control here.
constexpr const inline uint32_t ArtilleryMagicMinimumFlickDistanceRequired = 1901250; 
using namespace Arty;
struct FActionPatternParams
{
public:
	//this specifies a parametric bind's "preference" which will need to become an int for priority.
	//and if the binding consumes the input or passes it through.
	// an example is that we WILL want to say that holding down the trigger should be fired before
	// single press. actually, we might do pattern-priority rather than anything else.
	// hard to say. there is a trick here that could let us handle firing a diff ability if the ability is on cool down but I'm not borrowing trouble.
	bool preferToMatch = false;
	bool consumeInput = true;
	bool defaultBehavior = false;
	bool FiresCosmetics = false;

	FGunKey ToFire; //IT WAS NOT A MISTAKE. I AM A GENIUS.
	FActionBitMask ToSeek;
	InputStreamKey MyInputStream;
	FireControlKey MyOrigin;
	FActionPatternParams(const FActionBitMask ToSeek_In, FireControlKey MyOrigin_In, InputStreamKey MyInputStream_In, FGunKey Fireable) :
		ToSeek(ToSeek_In), MyOrigin(MyOrigin_In), MyInputStream(MyInputStream_In)
	{
		ToFire = Fireable;
	};

	friend uint32 GetTypeHash(const FActionPatternParams& Other)
	{
		// it's probably fine!
		return GetTypeHash(Other.ToFire) + GetTypeHash(Other.ToSeek);
	}
	
};

bool operator==(FActionPatternParams const& lhs, FActionPatternParams const& rhs) {
	return lhs.ToFire == rhs.ToFire;
}
#include "FActionPattern.h"

namespace Arty
{
	DECLARE_DELEGATE_TwoParams(FArtilleryFireGunFromDispatch, FGunKey GunID, bool InputAlreadyUsedOnce);
}
