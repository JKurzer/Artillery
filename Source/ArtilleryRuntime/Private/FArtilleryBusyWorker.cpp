#include "FArtilleryBusyWorker.h"

FArtilleryBusyWorker::FArtilleryBusyWorker() : running(false) {
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
				BristleconeControlStream.add(*((TheCone::Packet_tpl*)(packedInput))->GetPointerToElement(indexInput % 3),
					((TheCone::Packet_tpl*)(packedInput))->GetTransferTime());
				if (missedPrior)
				{
					BristleconeControlStream.add(
						*((TheCone::Packet_tpl*)(packedInput))->GetPointerToElement((indexInput - 1) % 3),
						((TheCone::Packet_tpl*)(packedInput))->GetTransferTime()
					);
					if (burstDropDetected)
					{
						//
						BristleconeControlStream.add(*((TheCone::Packet_tpl*)(packedInput))->GetPointerToElement((indexInput - 2) % 3),
							((TheCone::Packet_tpl*)(packedInput))->GetTransferTime());
					}
				}
				RemoteInput = true;
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
		}


	}
	return 0;
}

void FArtilleryBusyWorker::Exit() {
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Stopping Artillery thread."));
	Cleanup();
}

void FArtilleryBusyWorker::Stop() {
	UE_LOG(LogTemp, Display, TEXT("Artillery:BusyWorker: Stopping Artillery thread."));
	Cleanup();
}

void FArtilleryBusyWorker::Cleanup() {

	running = false;
}
