#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "FControllerState.h"
#include "SocketSubsystem.h"
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

private:
	void Cleanup();
	bool running;
};
