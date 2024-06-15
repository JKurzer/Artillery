#include "FArtilleryBusyWorker.h"
#include "Containers/TripleBuffer.h"

FArtilleryBusyWorker::FArtilleryBusyWorker() : running(false), TheTruthOfTheMatter(nullptr) {
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Constructing Artillery"));
}

FArtilleryBusyWorker::~FArtilleryBusyWorker() {
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Destructing Artillery"));
}

bool FArtilleryBusyWorker::Init() {
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Initializing Artillery thread"));
	running = true;
	return true;
}

uint32 FArtilleryBusyWorker::Run() {
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Running Artillery thread"));
	if(TheTruthOfTheMatter == nullptr)
	{
		//oh no you bloody don't.
#ifdef UE_BUILD_SHIPPING
		return -1;
#else
		throw; // this is a BUG. A BAD ONE. 
#endif
	}
	bool missedPrior = false;

	bool burstDropDetected = false;
	while (running) {
		TheCone::PacketElement current = 0;
		bool RemoteInput = false;

		while (InputRingBuffer != nullptr && !InputRingBuffer.Get()->IsEmpty())
		{
			const TheCone::Packet_tpl* packedInput = InputRingBuffer.Get()->Peek();
			auto indexInput = packedInput->GetCycleMeta() + 3; //faster than 3xabs or a branch.
			{
				//unlike the old design, we use an array of inputs from first -> current
				//so we want to add oldest first, then next, then next.
				//we'll need to amend this to handle correct defaulting of missing input,
				//which we can detect by both cycle skips and arrival window misses.
				//we then need a way, during rollbacks, to perform the rewrite.
				//right now, we just wait until we get the remote input.
				//honestly, bristle is fast enough and reliable enough that this might be fine?
				//flats don't have a recovery window, anyway, but we do get two tries.
				// tho otoh, I'm still wondering if no recovery window was a good idea.
				// maybe just a window of two? I'll need to check the bandwidth.
				//right now, we don't batch into flats. we'll want to wrap this in ifdefs
				//instead of removing it, though, because I don't think fighting games will use batched flats
				//and I'm wondering how much use they are, personally
				if (missedPrior)
				{
					if (burstDropDetected)
					{
						//
						BristleconeControlStream.add(*((TheCone::Packet_tpl*)(packedInput))->GetPointerToElement((indexInput - 2) % 3),
							((TheCone::Packet_tpl*)(packedInput))->GetTransferTime());
					}
					BristleconeControlStream.add(
						*((TheCone::Packet_tpl*)(packedInput))->GetPointerToElement((indexInput - 1) % 3),
						((TheCone::Packet_tpl*)(packedInput))->GetTransferTime()
					);

				}
				BristleconeControlStream.add(*((TheCone::Packet_tpl*)(packedInput))->GetPointerToElement(indexInput % 3),
					((TheCone::Packet_tpl*)(packedInput))->GetTransferTime());
				
				RemoteInput = true; //we check for empty at the start of the while. no need to check again.
				InputRingBuffer.Get()->Dequeue();
			}
		}

		if (RemoteInput == true)
		{
			missedPrior = false;
			burstDropDetected = false;
		}
		else
		{
			if (burstDropDetected)
			{
				//add rolling average switch-over here
			}
			if (missedPrior)
			{
				burstDropDetected = true;
			}
			missedPrior = true;
		}

		//Snag the local input, if any. we don't actually need a while here anymore PROBABLY but. :)
		while (InputSwapSlot != nullptr && !InputSwapSlot.Get()->IsEmpty())
		{
			current = *InputSwapSlot.Get()->Peek();
			CablingControlStream.add(current);
			InputSwapSlot.Get()->Dequeue();
			//UnrealEditor_ArtilleryRuntime!FArtilleryBusyWorker::Run() [C:\Users\jakek\Documents\Unreal Projects\Bristle54\Plugins\Artillery\Source\ArtilleryRuntime\Private\FArtilleryBusyWorker.cpp:98]
			//UnrealEditor_Core!FRunnableThreadWin::Run() [D:\build\++UE5\Sync\Engine\Source\Runtime\Core\Private\Windows\WindowsRunnableThread.cpp:149]
			//my guess is that input is getting sandblasted or never got initialized due to some old handling in bristle.
		}

		
		/*
		* 
		* Movement processing calls go here. before pattern matching.
		* 
		* 
		*/

#define ARTILLERY_FIRE_CONTROL_MACHINE_HANDLING (false)
//Patterns run here. The thread queues the fires. the dispatch fires them via the machines on the gamethread.
		//in order. that matters a tad.
		
		/*
		* 
		* Pattern matching will go here.
		* artillery fire control machines bind their input search patterns to this thread
		* hence their names, they're effectively facades for a larger finite state machine 
		* found here.
		* 
		* this emits events that the artillery fire control machines use as triggers
		* 
		* 
		*/

		//Per input stream, run their patterns here. god in heaven.
		//START HERE AND WORK YOUR WAY OUT TO UNDERSTAND PATTERNS, MATCHING, AND INPUT FLOW.
		//BristleconeControlStream.MyPatternMatcher->runOneFrameWithSideEffects(true, 0, 0); // those zeroes will stay here until we have resim.
		//This performs a copy of the map, I think. I HOPE it does a move, but I doubt it.
		auto refDangerous_LifeCycleManaged_TripleBuffered
		= TheTruthOfTheMatter->GetWriteBuffer();
		refDangerous_LifeCycleManaged_TripleBuffered.Reset();
		//today's sin is PRIDE, bigbird!
		CablingControlStream.MyPatternMatcher->runOneFrameWithSideEffects(
			true,
			0,
			0,
			CablingControlStream.highestInput - 1,
			refDangerous_LifeCycleManaged_TripleBuffered
			); // this looks wrong but I'm pretty sure it ain' since we reserve highest.

		//TODO: verify if this is needed.
		refDangerous_LifeCycleManaged_TripleBuffered.Sort();
		TheTruthOfTheMatter->SwapWriteBuffers();
		/*
		* 
		* Does rollback & reconciliation go here?
		* 
		* 
		*/

		/*
		*
		* Jolt will go here. No point in updating if we need to reconcile first.
		* Note: We also have Iris performing intermittent state stomps to recover from more serious desyncs.
		* Ultimately, rollback can never solve everything. The windows just get too wide.
		* 
		*/
		std::this_thread::yield();
		/*
		* 
		* Or does rollback go here? Or do we want it in a separate thread? This one's getting crowded.
		* I think jolt perf will be good enough....? I don't anticipate more than 200 bodies at a time.
		* 
		*/
	}
	return 0;
}

void FArtilleryBusyWorker::Exit() {
	UE_LOG(LogTemp, Display, TEXT("ARTILLERY OFFLINE."));
	Cleanup();
}

void FArtilleryBusyWorker::Stop() {
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Stopping Artillery thread."));
	Cleanup();
}

void FArtilleryBusyWorker::Cleanup() {

	running = false;
}
