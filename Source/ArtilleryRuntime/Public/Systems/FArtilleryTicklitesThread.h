﻿#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include <Ticklite.h>
#include "BristleconeCommonTypes.h"

//this is a busy-style thread, which runs preset bodies of work in a specified order. Generally, the goal is that it never
//actually sleeps. In fact, it yields rather than sleeps, in general operation.
// 
// This is similar to but functionally very different from a work-stealing or task model like what we see in rust.
// This thread runs ticklites, which are simple functions that satisfy the following properties:
// They are order insensitive. Surprisingly, most things are.
// They do not run the tick they are applied.
// They operate only on gameplay tags, abilities, and transforms.
// Anything that would normally be a ObjectPtr, SharedPtr, or Reference of any kind should be replaced by a key unless you are absolutely sure that object has a match-duration lifecycle.
// This is because expired ticklikes may be reactivated during rollback, as one would expect. Because we reverse attribute and tag _state_, those will be _ticked_ as normal.s 
// They modify only gameplay tags and abilities as of this writing. Transforms may become possible with Jolt.
// In the interim, we will support velocity change events submitted to the busyworker. This may be the permanent system.
// In general, if it cannot be accessed through Artillery, Thistle, or Sunflower, do not use it.
// The gameplay tags and abilities they operate on are owned and managed by the artillery system.
// They run on this thread, rather than the gameplay thread.
// They have either a duration, a lifecycle owner, or are created by a gameplay experience that serves as their lifecycle owner.*
//  
// * This is not enforced.
// 
// Unlike OnTick, which can't make even rudimentary guarantees about when or how often it will run,
// Ticklikes run on the artillery tick rather than on the UE frame. They also run on this thread,
// rather than the gamethread. FInally, and most importantly Ticklikes can be attached to anything,
// not just actors. this allows one ticklike to move multiple projectiles, affect multiple actors,
// or do things like batch process conditions such as frozen. This sounds like a performance optimization,
// and in many ways it is, but it also creates a far easier usage semantic than gameplay effects which often struggle
// despite being nominally more flexible.
// 
// It also means they must be written in C++ for the time being. Hopefully, we can relax this restriction.
// Fortunately, because they are NOT assets, it is trivial to make them data driven. This means we need quite few
// of them overall, and that they can be easily instanced and referenced from blueprint. 
// 
// Finally, Ticklikes may be used to trigger the firing of ArtilleryGuns on the next frame and
// any complex ticklike should probably work this way during prototyping. It does mean the earliest
// possible firing is 2 frames after application, but at 120ticks per second, that's 16ms.
//  
// As a result of being unable to fire the frame they are added, and unable to fire guns on the frame they fire,
// the fastest cadence we allow a ticklite to be checked at is 2. Cadences are aligned for entire groups.
// Do not rely on cadence to ensure ordering.
// 
// If ordering is mandatory, absolutely mandatory, start by using Phase. If that's not a strong enough guarantee,
// consider using either an ArtilleryAutoGun or triggering an ArtilleryGun from your ticklite. In general, though,
// effects like thorns should apply in the last phase. Phases are intended to order ticklikes relative to other ticklikes.
// Any additional ordering benefits they provide should be considered UB for the time being, and should not be relied on.
// 
//  Good luck, and may the force be with you.
template <typename UDispatch>
class FArtilleryTicklitesWorker : public FRunnable {

	//This isn't super safe but like busy worker, ticklites only runs in one spot.
	friend class UArtilleryDispatch;
	ArtilleryTime LocalNow;

	static const int GroupCount = 4;
	TickliteGroup ExecutionGroups[GroupCount];


	
	
	protected:
	TickliteBuffer QueuedAdds;
	
	bool TickliteAdd(TSharedPtr<TicklitePrototype> AllocatedTL,  TicklitePhase Group)
	{
		switch (Group)
		{
		case TicklitePhase::RECHARGE :
			{
				ExecutionGroups[0].Add(AllocatedTL);
				return true;
			}			
		case TicklitePhase::Early :
			{
				ExecutionGroups[1].Add(AllocatedTL);
				return true;
			}
		case TicklitePhase::Normal :
			{
				ExecutionGroups[2].Add( AllocatedTL);
				return true;
			}
		case TicklitePhase::Late :
			{
				ExecutionGroups[3].Add(AllocatedTL);
				return true;
			}
		}
		return false;
	}
	FSharedEventRef StartTicklitesSim;
	FSharedEventRef StartTicklitesApply;
	public:
	//Templating here is used to both make reparenting easier if needed later and to simplify our dependency tree
	UDispatch* DispatchOwner;
	FTransform3d& GetTransformShadowByObjectKey(ObjectKey Target, ArtilleryTime Now)
	{
		return DispatchOwner->GetTransformShadowByObjectKey(Target,  Now);
	}
	FArtilleryTicklitesWorker(): LocalNow(0), DispatchOwner(nullptr), running(false)
	{
		QueuedAdds = MakeShareable(new TickliteRequests(128));
	}

	void RequestAddTicklite(TSharedPtr<TicklitePrototype> ToAdd, TicklitePhase Group)
	{
		QueuedAdds->Enqueue(StampLiteRequest(ToAdd, Group));
	}
	
	inline ArtilleryTime GetShadowNow()
	const
	{
		return DispatchOwner->GetShadowNow();
	}

	inline AttrPtr GetAttrib(ObjectKey Target, AttribKey Attr)
	{
		return DispatchOwner->GetAttrib(Target, Attr);
	}

	virtual ~FArtilleryTicklitesWorker() override
	{
		UE_LOG(LogTemp, Display, TEXT("Artillery: Destructing SimTicklites thread."));
	};
	virtual bool QueueRollback()
	{
		//rollback is not implemented yet, but works by removing ticklikes added after the rollback's timestamp.
		//then adding back in any expired ticklikes that should be revived, clearing the current tick, and beginning resim.
		//Implementing this will not be easy, but it will suck a lot less than trying to do this with gameplay effects.
		//This is one reason we advocate STRONGLY for the use of KEYS over references, as references to memmory location
		//are not durable across rollbacks.
		throw; 
	}

	virtual bool Init() override
	{
		LocalNow = 0;
		UE_LOG(LogTemp, Display, TEXT("Artillery: Booting SimTicklites thread."));
		running = true;
		return true;
		
	}
	//TODO: ADD NULL GUARDS OR COPY. PREFER GUARD.
	void ApplyINE(TSharedPtr<TicklitePrototype>& x)
	{
		
		if( x->ShouldExpireTickable())
		{
			//TODO: swap from arrays to slab or true pool?
			//Can't implement until we're sure that they _tick_
		}
		else
		{
			x->ApplyTickable();
		}
	}

	//TODO: ADD NULL GUARDS OR COPY. PREFER GUARD.
	void CalcINE(TSharedPtr<TicklitePrototype>& x)
	{
		if( x->ShouldExpireTickable())
		{
			//TODO: swap from arrays to slab or true pool?
			//Can't implement until we're sure that they _tick_
		}
		else
		{
			x->CalculateTickable();
		}
	}

	//adding cadence is going to be quite annoying.
	virtual uint32 Run() override
	{
		while(running) {
			StartTicklitesSim->Wait();
			StartTicklitesSim->Reset();

			for(auto& Group : ExecutionGroups)
			{
				for(auto Tickable : Group)
				{
					CalcINE(Tickable);
				}
			}
			StartTicklitesApply->Wait();
			
			StartTicklitesApply->Reset(); // we can run long on sim, not on apply.
			for (auto& Group : ExecutionGroups)
			{
				for(auto Tickable : Group)
				{
					ApplyINE(Tickable);
				}
			}
			DispatchOwner->ApplyShadowTransforms();
			while(!QueuedAdds->IsEmpty())
			{
				const StampLiteRequest AddTup = *QueuedAdds->Peek();
				TickliteAdd(AddTup.Key, AddTup.Value);
				QueuedAdds->Dequeue();
			}
				
		}
	
		return 0;
	}

	virtual void Exit() override
	{
		UE_LOG(LogTemp, Display, TEXT("Artillery: Exiting SimTicklites thread."));
		running = false;
		Cleanup();
	}

	virtual void Stop() override
	{
		FRunnable::Stop();
	}



	
	
private:
	void Cleanup()
	{
		running = false;
	};
	bool running;
};