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
		UCanonicalInputStreamECS* MyBrother = GetWorld()->GetSubsystem<UCanonicalInputStreamECS>();
		ArtilleryAsyncWorldSim.ContingentInputECSLinkage = MyBrother;
		//IF YOU REMOVE THIS. EVERYTHING EXPLODE. IN A BAD WAY.
		//TARRAY IS A VALUE TYPE. SO IS TRIPLEBUFF I THINK.
		ArtilleryAsyncWorldSim.RawPtr_AbilitiesTripleBuffer = &RequestorQueue_Abilities_TripleBuffer;//OH BOY. REFERENCE TIME. GWAHAHAHA.
		
		WorldSim_Thread.Reset(FRunnableThread::Create(&ArtilleryAsyncWorldSim, TEXT("ARTILLERY_ONLINE.")));
	}


	// Q: how do we get PlayerKey?
	// ANS: Currently, we don't. 
	// controlStream = 
}

void UArtilleryDispatch::Deinitialize()
{

	Super::Deinitialize();
	ArtilleryAsyncWorldSim.Stop();
	//We have to wait.
	if(WorldSim_Thread.IsValid())
	{
		//if we don't wait, this will crash when the truth of the matter is referenced. That's just the facts.
		WorldSim_Thread->Kill(true);
	}
}

void UArtilleryDispatch::Tick(float DeltaTime)
{
	//Super::Tick(DeltaTime);
	RunGuns(); // ALL THIS WORK. FOR THIS?! (Okay, that's really cool)
}

TStatId UArtilleryDispatch::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UArtilleryDispatch, STATGROUP_Tickables);
}


FGunKey UArtilleryDispatch::GetGun(FString GunDefinitionID, FireControlKey MachineKey)
{
	//We know it. We have known it. We continue to know it.
	//See you soon, Chief.
	GunDefinitionID = GunDefinitionID.IsEmpty() ? "M6D" : GunDefinitionID; //joking aside, an obvious debug val is needed.
	FGunKey Key = FGunKey(GunDefinitionID, monotonkey++);
	if(PooledGuns.Contains(GunDefinitionID))
	{

		TSharedPtr<FArtilleryGun> repurposing = *PooledGuns.Find(GunDefinitionID);
		PooledGuns.RemoveSingle(GunDefinitionID, repurposing);
		repurposing->FArtilleryGunRebind(Key);
		GunByKey.Add(Key, repurposing);
	}
	else
	{
		TSharedPtr<FArtilleryGun> NewGun = MakeShareable(new FArtilleryGun(Key));
		GunByKey.Add(Key, NewGun);
	}
	return Key;	
}

//returns false if already released.
bool UArtilleryDispatch::ReleaseGun(FGunKey Key, FireControlKey MachineKey)
{
	//We know it. We have known it. We continue to know it.
	//See you soon, Chief.
	if(GunByKey.Contains(Key))
	{
		
		TSharedPtr<FArtilleryGun> tracker;
		GunByKey.RemoveAndCopyValue(Key, tracker);
		PooledGuns.Add(Key.GunDefinitionID, tracker);
		return true;
	}
	return false;	
}

void UArtilleryDispatch::QueueResim(FGunKey Key, Arty::ArtilleryTime Time)
{
	if (ActionsToReconcile && ActionsToReconcile.IsValid())
	{
		ActionsToReconcile->Enqueue(std::pair<FGunKey, Arty::ArtilleryTime>(Key, Time));
	}
}

void UArtilleryDispatch::RunGuns()
{

	//Sort is not stable. Sortedness appears to be lost for operations I would not expect.
	for (auto x : RequestorQueue_Abilities_TripleBuffer.Read())
	{
		auto fired =  GunToFiringFunctionMapping.Find(x.Value)->ExecuteIfBound(
			*GunByKey.Find(x.Value)
			, false);
		TotalFirings += fired;
	}
	RequestorQueue_Abilities_TripleBuffer.SwapReadBuffers();
}

//this needs work and extension.
//TODO: add smear support.
void UArtilleryDispatch::RunLocomotions()
{

	//Sort is not stable. Sortedness appears to be lost for operations I would not expect.
	for (auto x : RequestorQueue_Locomos_TripleBuffer.Read())
	{
		//execute if bound cannot be used with return values
		//because Unreal does not use the STL or did not when that code was written
		//so they don't have the easy elegant idiom of the Optional as readily.
		bool fired = ActorToLocomotionMapping.Find(x.parent)->
		Execute(
			 x.previousIndex,
			 x.currentIndex,
			 false,
			 false
			 );
		TotalFirings += fired;
	}
	RequestorQueue_Locomos_TripleBuffer.SwapReadBuffers();
}


void UArtilleryDispatch::RERunGuns()
{
	if (ActionsToReconcile && ActionsToReconcile.IsValid())
	{
		throw;
	}
}

void UArtilleryDispatch::RERunLocomotions()
{
	throw;
}

void UArtilleryDispatch::LoadGunData()
{
#if UE_BUILD_SHIPPING != 0
		throw;
#endif
	FString AccumulatePath = FPaths::Combine(FPaths::ProjectPluginsDir(), "Artillery", "Data", "GunData");
		
}

void UArtilleryDispatch::QueueFire(FGunKey Key, Arty::ArtilleryTime Time)
{
	if (ActionsToOrder && ActionsToOrder.IsValid())
	{
		ActionsToOrder->Enqueue(std::pair<FGunKey, Arty::ArtilleryTime>(Key, Time));
	}
}
