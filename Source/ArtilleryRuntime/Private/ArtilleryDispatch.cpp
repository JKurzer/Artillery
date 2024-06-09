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
	}
	UseNetworkInput.store(true);
	UBristleconeWorldSubsystem* MySquire = GetWorld()->GetSubsystem<UBristleconeWorldSubsystem>();
	InputRingBuffer = MakeShareable(new PacketQ(256));
	MySquire->QueueOfReceived = InputRingBuffer;
	UCablingWorldSubsystem* DirectLocalInputSystem = GetWorld()->GetSubsystem<UCablingWorldSubsystem>();
	InputSwapSlot = MakeShareable(new IncQ(256));
	DirectLocalInputSystem->DestructiveChangeLocalOutboundQueue(InputSwapSlot);
}

void UArtilleryDispatch::Deinitialize()
{

	Super::Deinitialize();
}

void UArtilleryDispatch::Tick(float DeltaTime)
{
	//Super::Tick(DeltaTime);
	TheCone::PacketElement current = 0;
	bool input = false;
	std::vector<TheCone::PacketElement> m = std::vector<TheCone::PacketElement>();

	if (UseNetworkInput.load())
	{
		while (InputRingBuffer != nullptr && !InputRingBuffer.Get()->IsEmpty())
		{
			const TheCone::Packet_tpl* packedInput = InputRingBuffer.Get()->Peek();
			auto indexInput = packedInput->GetCycleMeta() + 3; //faster than 3xabs or a branch.
			m.push_back(*((TheCone::Packet_tpl*)(packedInput))->GetPointerToElement(indexInput % 3));
			if (missedPrior)
			{
				m.push_back(*((TheCone::Packet_tpl*)(packedInput))->GetPointerToElement((indexInput - 1) % 3));
				if (burstDropDetected)
				{
					m.push_back(*((TheCone::Packet_tpl*)(packedInput))->GetPointerToElement((indexInput - 2) % 3));
				}
			}
			input = true;
			InputRingBuffer.Get()->Dequeue();
		}
	}
	else
	{
		while (InputSwapSlot != nullptr && !InputSwapSlot.Get()->IsEmpty())
		{
			current = *InputSwapSlot.Get()->Peek();
			//TODO, move this into unpackstick, replace int call with enum, make stick non-static instance-only
			m.push_back(current);
			input = true;
			InputSwapSlot.Get()->Dequeue();
		}
	}
	if (input == true)
	{
		for (int i = 0; i < m.size(); ++i)
		{
			// TODO push into stream
			DoLeftStickMove(m[i], m.size(), ratio);
		}
		missedPrior = false;
		burstDropDetected = false;
	}
	else
	{

		AddMovementInput(GetActorForwardVector(), previousY * ratio);
		AddMovementInput(GetActorRightVector(), previousX * ratio);
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
}

TStatId UArtilleryDispatch::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UArtilleryDispatch, STATGROUP_Tickables);
}

bool UArtilleryDispatch::registerPattern(TSharedPtr<FActionPattern> ToBind, FActionBitMask ToSeek, FGunKey ToFire)
{
	return false;
}

bool UArtilleryDispatch::removePattern(TSharedPtr<FActionPattern> ToBind, FActionBitMask ToSeek, FGunKey ToFire)
{
	return false;
}

FGunKey UArtilleryDispatch::getNewGunInstance(FString GunDefinitionID)
{
	return FGunKey();
}
