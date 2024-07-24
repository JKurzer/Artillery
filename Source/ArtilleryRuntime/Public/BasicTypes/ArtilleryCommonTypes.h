s// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
/**
 * Component for managing input streams in an ECS-like way, where any controller can request any stream.
 */


#include "FActionBitMask.h"
#include "BristleconeCommonTypes.h"
#include "Containers/TripleBuffer.h"

namespace Arty
{
	enum APlayer
	{
		CABLE = 1,
		ECHO = 2,
		TWO = 20,
		THREE = 30,
		FOUR = 40,
		FIVE = 50,
		SIX = 60,
		SEVEN = 70,
		EIGHT = 80
	};
	typedef TheCone::PacketElement INNNNCOMING;
	typedef uint32_t InputStreamKey;
	typedef APlayer PlayerKey;
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

namespace Arty
{
	typedef uint64_t ArtilleryDataSetKey;
	typedef ArtilleryDataSetKey ADSKey;
	typedef uint64_t TickliteKey;
	
	DECLARE_DELEGATE(CalculateTicklite);
	//performs the actual data transformations.
	DECLARE_DELEGATE(ApplyTicklite);
	//resets any data related to apply
	DECLARE_DELEGATE(ResetTicklike)
	DECLARE_DELEGATE_TwoParams(NormalTicklite, ActorKey, ADSKey);
	
	enum TicklitePhase
	{
		Early = 0,
		Normal = 1024,
		Late = 2048
	};
	enum TickliteCadence
	{
		Critical = 2,
		Tick = 4,
		Lite = 10,
		Slow = 60,
		VisiblySlow = 120
	};
	enum TickliteLifecycles
	{
		Dropdead, //expires on a timer
		OwnerTag, //while the owner has a tag. any key can own a ticklite but please don't use gunkeys. 
		Owner	  //when the owner is released (fiddly)
	};

	namespace Ticklites
	{
	struct TicklikeMemoryBlock
	{
		TickliteCadence Cadence = TickliteCadence::Lite;
		TicklitePhase RunGroup = TicklitePhase::Normal;
		void* Core;
		void* MemoryBlock;
		TickliteLifecycles Life = TickliteLifecycles::Dropdead;
		uint64_t MadeStamp = 0;
	};
	//in all its ugliness. i believe there's a newer, more elegant way to do this, but...
	struct TickLikePrototype : TicklikeMemoryBlock
	{
		
		virtual void CalculateTickable() = 0;
		virtual bool ShouldExpireTickable() = 0;
		virtual void ApplyTickable() = 0;
		virtual void ReturnToPool() = 0;
		virtual ~TickLikePrototype()
		{
			delete Core;
			delete MemoryBlock;
		};
	};
		
		//conserved attributes mean that we always have a shadow copy ready.
		// a successful Calculate function should reference the attribute not by most recent, but by exact index.
		//good support for this isn't in yet, but during our early work, the conserved attributes will still
		//protect us from partial memory commits and similar that might cause truly weird bugs.
		//instead, we may just run tickables "across" ticks right now. That's obviously bad, and I'm thinking
		//about solutions, but right now, what I've done is engineer some flex in so that when we understand
		//the best way forward on that front, we can cohere the design down. this follows our
		//measure, cut, fit, finish policy, as this is still in the _measure_ phase.
	template <typename T, typename MyTIO>
	struct Ticklite : public TickLikePrototype
	{

		using Ticklite_Impl = T;
		using Impl_InOut = MyTIO;
		void CalculateTickable()
		override
		{
			static_cast<Impl_InOut*>(MemoryBlock)->TICKLITE_StateReset();
			//as always, the use of keys over references will make rollback far far easier.
			//when performing operations here, do not expect floating point accuracy above 16 ulps.
			//If you do, you will get fucked sooner or later. I guarantee it.
			static_cast<Impl_InOut*>(MemoryBlock) = static_cast<Ticklite_Impl*>(Core)->TICKLITE_Calculate();
		}
		void ApplyTickable()
		override
		{
			static_cast<Ticklite_Impl*>(Core)->TICKLITE_Apply(
			static_cast<Impl_InOut*>(MemoryBlock)
			);
		}
		void ReturnToPool()
		override
		{
			static_cast<Ticklite_Impl*>(Core)->TICKLITE_CoreReset();
			static_cast<Impl_InOut*>(MemoryBlock)->TICKLITE_StateReset();
		}
		virtual bool ShouldExpireTickable() override
		{
			return static_cast<Ticklite_Impl*>(Core)->TICKLITE_CheckForExpiration();
		}

		virtual ~Ticklite()
		override
		{
			delete static_cast<Ticklite_Impl*>(Core);
			delete static_cast<Ticklite_Impl*>(MemoryBlock);
		}
		//unfortunately, there's not a good way to avoid this, because we
		//need to be able to _reverse_ expiration when rolling back.
	};
}

	typedef TArray<Ticklites::TickLikePrototype> TickliteGroup;
	typedef TArray<TPair<BristleTime,FGunKey>> EventBuffer;
	typedef TTripleBuffer<EventBuffer> BufferedEvents;
	typedef TPair<ArtilleryTime, Ticklites::TickLikePrototype> StampLitePair;
	typedef TArray<StampLitePair> TickliteBuffer;
	typedef TTripleBuffer<TickliteBuffer> BufferedTicklites;
}

//PATH TO DATA TABLES
//constexpr const FString GunsManifests = "";
//TODO: ALWAYS customize this to the sample-rate you select for cabling. ALWAYS. Or your game will feel Real Bad.
constexpr int ArtilleryInputSearchWindow = TheCone::BristleconeSendHertz;
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

static bool operator==(FActionPatternParams const& lhs, FActionPatternParams const& rhs) {
	return lhs.ToFire == rhs.ToFire;
}


