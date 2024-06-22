#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "FControllerState.h"
#include "SocketSubsystem.h"
#include "CanonicalInputStreamECS.h"
#include <thread>
#include "BristleconeCommonTypes.h"
#include "Containers/TripleBuffer.h"


//this is a busy-style thread, which runs preset bodies of work in a specified order. Generally, the goal is that it never
//actually sleeps. In fact, it yields rather than sleeps, in general operation.
// 
// This is similar to but functionally very different from a work-stealing or task model like what we see in rust.
// 
// 
// The artilleryworker needs to be kept fairly busy or it will melt one cpu core yield-cycling. to be honest, worth it.
// no, seriously. with all the other sacrifices we've made occupying one core with game-sim physics, reconciliation,
// rollbacks, and pattern matching is a pretty good bargain. we'll want to revisit this for servers, of course.
class FArtilleryBusyWorker : public FRunnable {
	public:
	FArtilleryBusyWorker();
	virtual ~FArtilleryBusyWorker() override;
	//This isn't super safe, but Busy Worker is used in exactly one place
	//and the dispatcher that owns this memory MUST manage this lifecycle.
	TTripleBuffer<TArray<LocomotionParams>>* RawPtr_LocoTripleBuffer;
	TTripleBuffer<TArray<TPair<BristleTime,FGunKey>>>* RawPtr_AbilitiesTripleBuffer;
	
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;

	

	//this is a hack and MIGHT be replaced with an ECS lookup
	//though the clarity gain is quite nice, and privileging Cabling makes sense
	TSharedPtr<ArtilleryControlStream>  CablingControlStream;
	TSharedPtr<ArtilleryControlStream> BristleconeControlStream;
	TheCone::RecvQueue InputRingBuffer;
	TheCone::SendQueue InputSwapSlot;
	UCanonicalInputStreamECS* ContingentInputECSLinkage;
	
	
private:
	void Cleanup();
	bool running;
};
