#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "FControllerState.h"
#include "SocketSubsystem.h"
#include "CanonicalInputStreamECS.h"
#include "BristleconeCommonTypes.h"


//this is a busy-style thread, which pulls work from each available component in a hierarchy,
//constantly executing until it finds no work remaining. Generally, the goal is that it never
//actually sleeps. 
//This worker consumes asynchronous input from bristlecone, prepares
class FArtilleryBusyWorker : public FRunnable {
public:
	FArtilleryBusyWorker();
	virtual ~FArtilleryBusyWorker() override;

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;

	//this is a hack and MIGHT be replaced with an ECS lookup
	//though the clarity gain is quite nice, and privileging Cabling makes sense
	ArtilleryControlStream CablingControlStream;
	TheCone::RecvQueue InputRingBuffer;
	TheCone::SendQueue InputSwapSlot;
	//this is a hack and will be replaced with an ECS lookup as each remote player will have 
	//a separate control stream for ease of use and my sanity.
	ArtilleryControlStream BristleconeControlStream;
private:
	void Cleanup();
	bool running;
};
