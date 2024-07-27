#pragma once
#include "ArtilleryCommonTypes.h"


namespace Ticklites
{
	//conserved attributes mean that we always have a shadow copy ready.
	// a successful Calculate function should reference the attribute not by most recent, but by exact index.
	//good support for this isn't in yet, but during our early work, the conserved attributes will still
	//protect us from partial memory commits and similar that might cause truly weird bugs.
	//instead, right now, we just run tickables "across" ticks right now.
	//This conforms with our measure, cut, fit, finish policy, as this is still in the _measure_ phase.
	template <typename YourImplementation,
	typename ParentThreadAnchor,
	typename YourThreadsafeState=YourImplementation,
	typename ExpirationPolicy=YourImplementation>
	struct Ticklite : public TickLikePrototype 
	{
		//Each class generated gets a unique static. Each kind of dispatcher will get a unique class.
		//TODO: If you run more than one of the parent threads, this gets unsafe. We don't so...
		//As is, it saves a huge amount of memory and indirection costs.
		static ParentThreadAnchor* ADispatch = nullptr; 
		using Ticklite_Impl = YourImplementation;
		using Impl_InOut = YourThreadsafeState;

		ArtilleryTime GetShadowNow()
		{
			return ADispatch->GetShadowNow();
		}
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
			return static_cast<ExpirationPolicy*>(Core)->TICKLITE_CheckForExpiration();
		}
		virtual bool OnExpireTickable()
		override
		{
			return static_cast<ExpirationPolicy*>(Core)->TICKLITE_OnExpiration();
		}

		virtual ~Ticklite()
		override
		{
			delete static_cast<Ticklite_Impl*>(Core);
			delete static_cast<Impl_InOut*>(MemoryBlock);
		}

		//You may wish to implement a ticklite that uses reference counting.
		//this DOES NOT. Ticklites and their implementations are assumed to have different lifecycles,
		//with the ticklite either dying at the same time, or dying first. Anything besides this will cause bugs.
		Ticklite(Ticklite_Impl* ImplInstance, Impl_InOut* MemoryInstance)
		{
			Core = ImplInstance;
			MemoryBlock = MemoryInstance;
		}
	};
}


