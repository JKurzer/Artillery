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
		): TL_Impl<ParentThreadAnchor>(Anchor), VelocityTarget(Target), TicksToSplitVelocityOver(Duration), TicksRemaining(Duration),
	PerTickVelocityToApply(VelocityVec::ZeroVector), Velocity(Velocity)
	{
	}
	void TICKLITE_StateReset()
	{
		PerTickVelocityToApply = PerTickVelocityToApply.ZeroVector;
	}
	void TICKLITE_Calculate()
	{
		PerTickVelocityToApply = Velocity/TicksToSplitVelocityOver;
	}
	void TICKLITE_Apply(FTLinearVelocity* SelfRef)
	{

	}
	void TICKLITE_CoreReset()
	{
		PerTickVelocityToApply = VelocityVec::ZeroVector;
		Velocity = VelocityVec::ZeroVector;
		TicksRemaining = 0; TicksToSplitVelocityOver = 0;
		VelocityTarget = 0;
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
typedef Ticklites::Ticklite<FTLinearVelocity<FArtilleryTicklitesWorker*>> TL_LinearVelocity;
