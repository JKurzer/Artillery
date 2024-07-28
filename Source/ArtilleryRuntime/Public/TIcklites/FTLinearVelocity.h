#pragma once
#include "Ticklite.h"
#include "ArtilleryDispatch.h"
#include "FArtilleryTicklitesThread.h"
#include "TL_Impl.h"



//A ticklite's impl component(s) must provide:
//TICKLITE_StateReset on the memory block aspect
//TICKLITE_Calculate on the impl aspect
//TICKLITE_Apply(MemoryBlock*) on the impl aspect, consuming the memory block aspect's state
//TICKLITE_CoreReset on the impl aspect
//TICKLITE_CheckForExpiration on the impl aspect
//TICKLITE_OnExpiration
template<typename ParentThreadAnchor>
class FTLinearVelocity : public TL_Impl<ParentThreadAnchor> /*Facaded*/
{
public:
	ObjectKey VelocityTarget;
	VelocityVec PerTickVelocityToApply;
	uint32 TicksToSplitVelocityOver;
	uint32 TicksRemaining; //TODO: make this rollback correctly.
	VelocityVec Velocity;
	FTLinearVelocity(ParentThreadAnchor* Anchor): TL_Impl<ParentThreadAnchor>(Anchor), VelocityTarget(0), TicksToSplitVelocityOver(1), TicksRemaining(0)
	{
	}

	FTLinearVelocity(ParentThreadAnchor* Anchor,
		ObjectKey Target,
		VelocityVec Velocity,
		uint32 Duration
		) : TL_Impl<ParentThreadAnchor>(Anchor), VelocityTarget(Target), TicksToSplitVelocityOver(Duration), TicksRemaining(Duration),
	PerTickVelocityToApply(Velocity/Duration), Velocity(Velocity)
	{
	}
	void TICKLITE_StateReset()
	{
	}
	void TICKLITE_Calculate()
	{
	}
	void TICKLITE_Apply(FTLinearVelocity* SelfRef)
	{
		ArtilleryTime Now = this->GetShadowNow();
		--TicksRemaining;
		FTransform3d& Transform = this->ADispatch->GetTransformShadowByObjectKey(VelocityTarget, Now);
		Transform.AddToTranslation(PerTickVelocityToApply);
	}
	void TICKLITE_CoreReset()
	{
		TicksRemaining = TicksToSplitVelocityOver;
	}

	bool TICKLITE_CheckForExpiration()
	{
		return TicksRemaining == 0;
	}

	void TICKLITE_OnExpiration()
	{
		//no op
	}
};
//behold!
// FTLinearVelocity<FArtilleryTicklitesWorker*>& would allow you to use a reference indirection here, I believe.
typedef Ticklites::Ticklite<FTLinearVelocity<FArtilleryTicklitesWorker*>> TL_LinearVelocity;
