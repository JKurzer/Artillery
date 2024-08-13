#include "FArtilleryBusyWorker.h"
#include "Containers/TripleBuffer.h"

FArtilleryBusyWorker::FArtilleryBusyWorker() : running(false), RequestorQueue_Abilities_TripleBuffer(nullptr)
{
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Constructing Artillery"));
}

FArtilleryBusyWorker::~FArtilleryBusyWorker()
{
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Destructing Artillery"));
}

bool FArtilleryBusyWorker::Init()
{
	//you cannot reorder these. it is a magic ordering put in place for a hack. 

	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Initializing Artillery thread"));
	running = true;
	return true;
}

void FArtilleryBusyWorker::RunStandardFrameSim(bool& missedPrior, uint64_t& currentIndexCabling,
                                               bool& burstDropDetected, TheCone::PacketElement& current,
                                               bool& RemoteInput) const
{
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

	if (InputSwapSlot != nullptr && !InputSwapSlot.Get()->IsEmpty())
	{
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

		//Pattern matchers match, set events, and then those events are handed to the dispatch for now.
		//gradually, we'll be able to run more and more of them on this thread, freeing us from the tyranny.
		//do not refactor to auto. it will break.
		MovementBuffer& refDangerous_LifeCycleManaged_Loco_TripleBuffered
			= RequestorQueue_Locomos_TripleBuffer->GetWriteBuffer();

		//Per input stream, run their patterns here. god in heaven.
		//START HERE AND WORK YOUR WAY OUT TO UNDERSTAND PATTERNS, MATCHING, AND INPUT FLOW.
		//BristleconeControlStream.MyPatternMatcher->runOneFrameWithSideEffects(true, 0, 0); // those zeroes will stay here until we have resim.
		//This performs a copy of the map, I think. I HOPE it does a move, but I doubt it.
		EventBuffer& refDangerous_LifeCycleManaged_Abilities_TripleBuffered
			= RequestorQueue_Abilities_TripleBuffer->GetWriteBuffer();

		
		//today's sin is PRIDE, bigbird!
		for (int i = currentIndexCabling; i < CablingControlStream->highestInput; ++i)
		{
			//TODO: does this leak memory?
			auto actor = CablingControlStream->GetActorByInputStream();
			if(actor)
			{
				refDangerous_LifeCycleManaged_Loco_TripleBuffered.Add(
					LocomotionParams(
						CablingControlStream->peek(i)->SentAt,
						actor,
						*CablingControlStream->peek(i - 1),
						*CablingControlStream->peek(i)
					)
				);

				CablingControlStream->MyPatternMatcher->runOneFrameWithSideEffects(
					true,
					0,
					0,
					i,
					refDangerous_LifeCycleManaged_Abilities_TripleBuffered
				); // this looks wrong but I'm pretty sure it ain' since we reserve highest.

			}
				//even if this doesn't get played for some reason, this is the last chance we've got to make a
				//truly informed decision about the matter. By the time we reach the dispatch system, that chance is gone.
				//Better to skip a cosmetic once in a while than crash the game.
				CablingControlStream->get(CablingControlStream->highestInput - 1)->RunAtLeastOnce = true;
			
		}

		refDangerous_LifeCycleManaged_Loco_TripleBuffered.Sort();
		refDangerous_LifeCycleManaged_Abilities_TripleBuffered.Sort();
		if (RequestorQueue_Abilities_TripleBuffer->IsDirty() == false)
		{
			RequestorQueue_Abilities_TripleBuffer->SwapWriteBuffers();
		}
		if (RequestorQueue_Locomos_TripleBuffer->IsDirty() == false)
		{
			RequestorQueue_Locomos_TripleBuffer->SwapWriteBuffers();
		}
	}
}

uint32 FArtilleryBusyWorker::Run()
{
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Running Artillery thread"));
	if (RequestorQueue_Abilities_TripleBuffer == nullptr)
	{
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
	bool sent = false;
	//TODO: remember why this needs to be an int. Overflow would take a match running for 1000 hours.
	//if you wanna use this for a really long lived session, you'll need to fix it.
	int seqNumber = 0;
	//Hi! Jake here! Reminding you that this will CYCLE
	//That's known. Isn't that fun? :) Don't reorder these, by the way.
	uint32_t LastIncrementWindow = ContingentInputECSLinkage->Now();
	uint32_t lsbTime = ContingentInputECSLinkage->Now();
	constexpr uint32_t sampleHertz = TheCone::CablingSampleHertz;
	constexpr uint32_t sendHertz = LongboySendHertz;
	constexpr uint32_t sendHertzFactor = sampleHertz / sendHertz;
	constexpr uint32_t periodInNano = 1000000 / sampleHertz; //swap to microseconds. standardizing.


	//we can now start the sim. we latch only on the apply step.
	StartTicklitesSim->Trigger();
	
	while (running)
	{
		if (!sent &&
			(
				InputRingBuffer != nullptr && !InputRingBuffer.Get()->IsEmpty()
				|| seqNumber % sendHertzFactor == 0 //last chance. Not good.
			)
		)
		{
			//using k-1 here causes an off-by-one error that causes input to get echoed like mad.
			currentIndexCabling = CablingControlStream->highestInput;
			currentIndexBristlecone = BristleconeControlStream->highestInput;
			TheCone::PacketElement current = 0;
			bool RemoteInput = false;

			RunStandardFrameSim(missedPrior, currentIndexCabling, burstDropDetected, current, RemoteInput);
			/*
			*
			* Jolt will go here? No point in updating if we need to reconcile first.
			* Note: We also have Iris performing intermittent state stomps to recover from more serious desyncs.
			* Ultimately, rollback can never solve everything. The window's just get too wide.
			*/
			sent = true;
			TickliteNow = ContingentInputECSLinkage->Now(); // this updates ONCE PER CYCLE. ONCE. THIS IS INTENDED.
			StartTicklitesApply->Trigger();
		}

		//unlike cabling, we do our time keeping HERE. It may be worth switching cabling to also follow this.
		//though if we end up using frameworks where the poll isn't free, we'll get dorked for doing it this way.
		//Increment window is still used to ensure we have at least two milliseconds to run, though.
		if ((LastIncrementWindow + periodInNano) <= lsbTime)
		{
			LastIncrementWindow = lsbTime;
			if ((seqNumber % sendHertzFactor) == 0)
			{
				sent = false;
			}
			++seqNumber;
		}
		std::this_thread::yield();
		lsbTime = ContingentInputECSLinkage->Now();
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
