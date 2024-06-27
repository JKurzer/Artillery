#include "FArtilleryBusyWorker.h"
#include "Containers/TripleBuffer.h"

FArtilleryBusyWorker::FArtilleryBusyWorker() : running(false), SPtrEventsBuffer(nullptr)
{
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Constructing Artillery"));
}

FArtilleryBusyWorker::~FArtilleryBusyWorker()
{
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Destructing Artillery"));
}

bool FArtilleryBusyWorker::Init()
{
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Initializing Artillery thread"));
	running = true;
	return true;
}

//void ABristle54Character::Tick(float DeltaSeconds) needs IMMEDIATE revision, as this will eat some of the input.
uint32 FArtilleryBusyWorker::Run()
{
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Running Artillery thread"));
	if (SPtrEventsBuffer == nullptr)
	{
		//oh no you bloody don't.
#ifdef UE_BUILD_SHIPPING
		return -1;
#else
		throw; // this is a BUG. A BAD ONE. 
#endif
	}
	bool missedPrior = false;
	uint64_t currentIndexCabling = 0;
	uint64_t currentIndexBristlecone = 0;
	bool burstDropDetected = false;
	//you cannot reorder these. it is a magic ordering put in place for a hack. 
	CablingControlStream = ContingentInputECSLinkage->getNewStreamConstruct();
	BristleconeControlStream = ContingentInputECSLinkage->getNewStreamConstruct();
	while (running)
	{
		currentIndexCabling = CablingControlStream->highestInput - 1;
		currentIndexBristlecone = BristleconeControlStream->highestInput - 1;
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
						BristleconeControlStream->Add(
							*((TheCone::Packet_tpl*)(packedInput))->GetPointerToElement((indexInput - 2) % 3),
							((TheCone::Packet_tpl*)(packedInput))->GetTransferTime());
					}
					BristleconeControlStream->Add(
						*((TheCone::Packet_tpl*)(packedInput))->GetPointerToElement((indexInput - 1) % 3),
						((TheCone::Packet_tpl*)(packedInput))->GetTransferTime()
					);
				}
				BristleconeControlStream->Add(
					*((TheCone::Packet_tpl*)(packedInput))->GetPointerToElement(indexInput % 3),
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


		//TODO: Figure out if this needs to wrap it all. p sure it does.
		//though it's probably more elegant and faster to index over the control streams
		while (InputSwapSlot != nullptr && !InputSwapSlot.Get()->IsEmpty())
		{
			current = *InputSwapSlot.Get()->Peek();
			CablingControlStream->Add(current);

			InputSwapSlot.Get()->Dequeue();
		}



#define ARTILLERY_FIRE_CONTROL_MACHINE_HANDLING (false)
		//First, locomotions are pushed.
		//Patterns run here. The thread queues the locomotions and fires.
		//the dispatch performs the locomotion operations. THEN
		//movement abilities (MAYBE)
		//THEN
		//the dispatch fires guns via the machines on the gamethread.
		//in order. that matters a tad.

		/*
		* 
		* artillery fire control machines bind their input search patterns to this thread
		* hence their names, they're effectively facades for a larger finite state machine 
		* found here.
		* 
		* this emits events that the artillery fire control machines use as triggers
		* 
		* 
		*/
		auto refDangerous_LifeCycleManaged_Loco_TripleBuffered
			= SPtrMoveBuffer->GetWriteBuffer();
		refDangerous_LifeCycleManaged_Loco_TripleBuffered.Reset();


		//Per input stream, run their patterns here. god in heaven.
		//START HERE AND WORK YOUR WAY OUT TO UNDERSTAND PATTERNS, MATCHING, AND INPUT FLOW.
		//BristleconeControlStream.MyPatternMatcher->runOneFrameWithSideEffects(true, 0, 0); // those zeroes will stay here until we have resim.
		//This performs a copy of the map, I think. I HOPE it does a move, but I doubt it.
		auto refDangerous_LifeCycleManaged_Abilities_TripleBuffered
			= SPtrEventsBuffer->GetWriteBuffer();
		refDangerous_LifeCycleManaged_Abilities_TripleBuffered.Reset();

		//today's sin is PRIDE, bigbird!
		for (int i = currentIndexCabling; i < CablingControlStream->highestInput; ++i)
		{
			//TODO: refDangerous_LifeCycleManaged_Loco_TripleBuffered work goes here
			
			CablingControlStream->MyPatternMatcher->runOneFrameWithSideEffects(
				true,
				0,
				0,
				i,
				refDangerous_LifeCycleManaged_Abilities_TripleBuffered
			); // this looks wrong but I'm pretty sure it ain' since we reserve highest.

			//TODO: does this leak memory?
			refDangerous_LifeCycleManaged_Loco_TripleBuffered.Add(
			LocomotionParams(
					CablingControlStream->peek(i)->SentAt,
					CablingControlStream->GetActorByInputStream(),
					*CablingControlStream->peek(i-1),
					*CablingControlStream->peek(i)
					)
				)
			;
			//even if this doesn't get played for some reason, this is the last chance we've got to make a
			//truly informed decision about the matter. By the time we reach the dispatch system, that chance is gone.
			//Better to skip a cosmetic once in a while than crash the game.
			CablingControlStream->get(CablingControlStream->highestInput - 1)->RunAtLeastOnce = true;
		}

		refDangerous_LifeCycleManaged_Loco_TripleBuffered.Sort();
		refDangerous_LifeCycleManaged_Abilities_TripleBuffered.Sort();
		SPtrEventsBuffer->SwapWriteBuffers();
		SPtrMoveBuffer->SwapWriteBuffers();
		/*
		* 
		* Does rollback & reconciliation go here?
		* 
		* 
		*/

		/*
		*
		* Jolt will go here? No point in updating if we need to reconcile first.
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

void FArtilleryBusyWorker::Exit()
{
	UE_LOG(LogTemp, Display, TEXT("ARTILLERY OFFLINE."));
	Cleanup();
}

void FArtilleryBusyWorker::Stop()
{
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Stopping Artillery thread."));
	Cleanup();
}


void FArtilleryBusyWorker::Cleanup()
{
	running = false;
}
