#include "FArtilleryBusyWorker.h"

FArtilleryBusyWorker::FArtilleryBusyWorker() : running(false){
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
	while (running) {
		
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
