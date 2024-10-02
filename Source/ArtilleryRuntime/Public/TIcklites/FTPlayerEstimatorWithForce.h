#pragma once
#include "Ticklite.h"
#include "ArtilleryDispatch.h"
#include "FArtilleryTicklitesThread.h"
#include "ArtilleryBPLibs.h"



//A ticklite's impl component(s) must provide:
//TICKLITE_StateReset on the memory block aspect
//TICKLITE_Calculate on the impl aspect
//TICKLITE_Apply on the impl aspect, consuming the memory block aspect's state
//TICKLITE_CoreReset on the impl aspect
//TICKLITE_CheckForExpiration on the impl aspect
//TICKLITE_OnExpiration

class FTPlayerEstimatorWithForce : public UArtilleryDispatch::TL_ThreadedImpl /*Facaded*/
{
public:
	FSkeletonKey VelocityTarget;
	VelocityVec PerTickVelocityToApply;
	uint32 TicksToSplitVelocityOver;
	uint32 TicksRemaining; //TODO: make this rollback correctly.
	VelocityVec Velocity;
	VelocityVec EstimatedDirection;
	VelocityVec Force;
	FTPlayerEstimatorWithForce(): TL_ThreadedImpl(), VelocityTarget(0), TicksToSplitVelocityOver(1), TicksRemaining(0), Force(), EstimatedDirection()
	{
	}

	FTPlayerEstimatorWithForce(
		FSkeletonKey Target,
		VelocityVec Velocity,
		uint32 Duration
		) : VelocityTarget(Target), PerTickVelocityToApply(Velocity/Duration), TicksToSplitVelocityOver(Duration),
	TicksRemaining(Duration), Velocity(Velocity), Force(), EstimatedDirection()
	{
	}
	void TICKLITE_StateReset()
	{
	}
	void TICKLITE_Calculate()
	{
		UArtilleryLibrary::K2_GetPlayerDirectionEstimator(EstimatedDirection);
		EstimatedDirection.Normalize();
		auto Input = PerTickVelocityToApply.GetSafeNormal2D();
		Force = ((Input + Input + EstimatedDirection) /3 ).GetSafeNormal() *  PerTickVelocityToApply.Length();
	}
	//this isn't quite right. we should calculate the component in calculate
	//but for now, this is good enough for testing.
	void TICKLITE_Apply()
	{
		ArtilleryTime Now = this->GetShadowNow();
		--TicksRemaining;
		FBLet GameSimPhysicsObject = this->ADispatch->GetFBLetByObjectKey(VelocityTarget, Now);
		if(GameSimPhysicsObject)
		{
			FBarragePrimitive::ApplyForce(Force, GameSimPhysicsObject);
		}
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
typedef Ticklites::Ticklite<FTPlayerEstimatorWithForce> TL_PlayerDirectedForce;
