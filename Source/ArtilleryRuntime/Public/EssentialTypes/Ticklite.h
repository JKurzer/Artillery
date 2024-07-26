#pragma once
#include "ArtilleryCommonTypes.h"
#include "ArtilleryDispatch.h"


namespace Ticklites
{
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

		virtual void CalculateTickable()
		override
		{
			static_cast<Impl_InOut*>(MemoryBlock)->TICKLITE_StateReset();
			//as always, the use of keys over references will make rollback far far easier.
			//when performing operations here, do not expect floating point accuracy above 16 ulps.
			//If you do, you will get fucked sooner or later. I guarantee it.
			static_cast<Impl_InOut*>(MemoryBlock) = static_cast<Ticklite_Impl*>(Core)->TICKLITE_Calculate();
		}

		virtual void ApplyTickable()
		override
		{
			static_cast<Ticklite_Impl*>(Core)->TICKLITE_Apply(
				static_cast<Impl_InOut*>(MemoryBlock)
			);
		}

		virtual void ReturnToPool()
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
	
	static UArtilleryDispatch* GADispatch = nullptr;
}

namespace Arty
{
	typedef TPair<ArtilleryTime, TickLikePrototype> StampLitePair;
	typedef TArray<TickLikePrototype> TickliteGroup;
	typedef TArray<StampLitePair> TickliteBuffer;
	typedef TTripleBuffer<TickliteBuffer> BufferedTicklites;
	//Ever see the motto of the old naval railgun project? I won't spoil it for you.
	typedef FVector3d VelocityVec;
	typedef TTuple<ArtilleryTime, ObjectKey, VelocityVec> VelocityEvent;

	typedef TCircularQueue<VelocityEvent> VelocityStack;
	typedef TSharedPtr<VelocityStack> VelocityEP; //event pump, if you must know.
}
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

	inline void AddVelocity(ObjectKey Target, VelocityVec Velocity)
	{
		//TODO: make this go away before it crashes, cause it can crash. Tick Tock, Mr. Wick.
		if (Ticklites::GADispatch!= nullptr)
		{
		}
	}

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
