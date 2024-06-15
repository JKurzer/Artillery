// Fill out your copyright notice in the Description page of Project Settings.


#include "ArtilleryDispatch.h"

void UArtilleryDispatch::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("ArtilleryDispatch:Subsystem: Online"));
}

void UArtilleryDispatch::OnWorldBeginPlay(UWorld& InWorld)
{
	if ([[maybe_unused]] const UWorld* World = InWorld.GetWorld()) {
		UE_LOG(LogTemp, Warning, TEXT("ArtilleryDispatch:Subsystem: World beginning play"));
		// getting input from Bristle
		UseNetworkInput.store(true);
		UBristleconeWorldSubsystem* MySquire = GetWorld()->GetSubsystem<UBristleconeWorldSubsystem>();
		ArtilleryAsyncWorldSim.InputRingBuffer = MakeShareable(new PacketQ(256));
		MySquire->QueueOfReceived = ArtilleryAsyncWorldSim.InputRingBuffer;
		UCablingWorldSubsystem* DirectLocalInputSystem = GetWorld()->GetSubsystem<UCablingWorldSubsystem>();
		ArtilleryAsyncWorldSim.InputSwapSlot = MakeShareable(new IncQ(256));
		DirectLocalInputSystem->DestructiveChangeLocalOutboundQueue(ArtilleryAsyncWorldSim.InputSwapSlot);
		WorldSim_Thread.Reset(FRunnableThread::Create(&ArtilleryAsyncWorldSim, TEXT("ARTILLERY ONLINE.")));
		ArtilleryAsyncWorldSim.Exit();
	}


	// Q: how do we get PlayerKey?
	// ANS: Currently, we don't. 
	// controlStream = 
}

void UArtilleryDispatch::Deinitialize()
{

	Super::Deinitialize();
}

void UArtilleryDispatch::Tick(float DeltaTime)
{
	//Super::Tick(DeltaTime);

}

TStatId UArtilleryDispatch::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UArtilleryDispatch, STATGROUP_Tickables);
}


FGunKey UArtilleryDispatch::GetNewGunKey(FString GunDefinitionID, FireControlKey MachineKey)
{
	//We know it. We have known it. We continue to know it.
	//See you soon, Chief.
	GunDefinitionID = GunDefinitionID.IsEmpty() ? "M6D" : GunDefinitionID; //joking aside, an obvious debug val is needed.
	FGunKey Key = FGunKey(GunDefinitionID, monotonkey++);
	
	GunToMachineMapping.emplace(Key, MachineKey);
	return Key;	
}
