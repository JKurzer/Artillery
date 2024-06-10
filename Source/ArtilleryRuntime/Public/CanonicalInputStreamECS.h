// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "Templates/SubclassOf.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "AttributeSet.h"
#include "Containers/CircularBuffer.h"
#include "BristleconeCommonTypes.h"
#include "UBristleconeWorldSubsystem.h"
#include <optional>
#include <unordered_map>
#include <ArtilleryShell.h>
#include "CanonicalInputStreamECS.generated.h"



/**
 * Component for managing input streams in an ECS-like way, where any controller can request any stream.
 */
typedef TheCone::PacketElement INNNNCOMING;
typedef uint32_t InputStreamKey;
typedef uint32_t PlayerKey;
typedef uint32_t ActorKey;

//TODO: finish adding the input streams, replace the local handling in Bristle54 character with references to the input stream ecs
//TODO: begin work on the conceptual frame for reconciling and assessing what input does and does not exist.
//AXIOMS: Input we send is real. Input we get in the batches represents the full knowledge of the server.
//AXIOM: A missed batch means we could be starting to desynchronize.
//AXIOM: Bristlecone Time + the fixed cadencing allows us to know when something SHOULD arrive.
//FACT: A batch missing input of OURS that we believe should be there could mean we are starting to desynchronize.
//FACT: A batch containing input of OURS that we believe should have been in an EARLIER batch could mean we are starting to desynchronize.
//FACT: The server will always have less of our input than we do.
//FACT: We may see input from other players before the server processes it.
//FACT: However, if it's missing from a batch, the server hasn't seen it yet.
//FACT: We may get server update pushes older than our input and/or older than the newest batch we have.
//FACT: We may have different orderings.
//FACT: we can determine the correct ordering and we all know the correct ordering of all input that made it into batches.

//Notes:
/*
Multiversus just relays input, it's all deterministic, and it records it.
At the end of the game, if clients disagree, it spins up a simulation and replays the inputs,
and uses that for the outcome (and presumably flags whoever disagreed for statistical detection).
*/


UCLASS()
class ARTILLERYRUNTIME_API UCanonicalInputStreamECS : public UTickableWorldSubsystem
{
	GENERATED_BODY()

/**
 * Conserved input streams record their last 8,192 inputs. Yes, that's a few megs across all streams. No, it doesn't really seem to matter.
 * Currently, this is for debug purposes, but we can use it with some additional features to provide a really expressive
 * model for rollback at a SUPER granular level if needed. UE has existing rollback tech, though so...
 * 
 * These streams are designed for single reader, single producer, but it is hypothetically possible to have a format where you do
 * single producer, single consumer, multiple observer. I don't recommend that, due to the need to mark records as played for cosmetics.
 * 
 * It is possible that an observer might end up with a stale view of if a record's cosmetic effects have been applied, in this circumstance.
 * This is DONE DURING THE GET. That can lead to an unholy mess. If you need an observer, ensure that it does not regard cosmetics as important
 * AND use a PEEK.
 */
public:
	ArtilleryTime Now()
	{
		return MySquire->Now();
	};
	static const uint32_t InputConservationWindow = 8192;
	static const uint32_t AddressableInputConservationWindow = InputConservationWindow - (2 * TheCone::LongboySendHertz);
	friend class FArtilleryBusyWorker;
	class ARTILLERYRUNTIME_API FConservedInputStream
	{
	friend class FArtilleryBusyWorker;
	public:
		TCircularBuffer<FArtilleryShell> CurrentHistory = TCircularBuffer<FArtilleryShell>(InputConservationWindow); //these two should be one buffer of a shared type, but that makes using them harder
		InputStreamKey MyKey; //in case, god help us, we need a lookup based on this for something else. that should NOT happen.


		//Correct usage procedure is to null check then store a copy.
		//Failure to follow this procedure will lead to eventual misery.
		//This has a side-effect of marking the record as played at least once.
		std::optional<FArtilleryShell> get(uint64_t input)
		{
			// the highest input is a reserved write-slot.
			//the lower bound here ensures that there's always minimum two seconds worth of memory separating the readers
			//and the writers. How safe is this? It's not! But it's insanely fast. Enjoy, future jake!
			//TODO: Refactor this to use an atomic int instead of this hubristic madness.
			if (input >= highestInput || (highestInput - input) > AddressableInputConservationWindow)
			{
				return std::optional<FArtilleryShell>(
					std::nullopt
				);
			}
			else {
				CurrentHistory[input].RunAtLeastOnce = true; //this is the only risky op in here from a threading perspective.
				return std::optional<FArtilleryShell>(CurrentHistory[input]);
			}
		};

		//THE ONLY DIFFERENCE WITH PEEK IS THAT IT DOES NOT SET RUNATLEASTONCE.
		//PEEK IS PROVIDED ONLY AS AN EMERGENCY OPTION, AND IF YOU FIND YOURSELF USING IT
		//MAY GOD HAVE MERCY ON YOUR SOUL.
		std::optional<FArtilleryShell> peek(uint64_t input)
		{
			// the highest input is a reserved write-slot.
			//the lower bound here ensures that there's always minimum two seconds worth of memory separating the readers
			//and the writers. How safe is this? It's not! But it's insanely fast. Enjoy, future jake!
			//TODO: Refactor this to use an atomic int instead of this hubristic madness.
			if (input >= highestInput || (highestInput - input) > AddressableInputConservationWindow)
			{
				return std::optional<FArtilleryShell>(
					std::nullopt
				);
			}
			else {
				return std::optional<FArtilleryShell>(CurrentHistory[input]);
			}
		};

		InputStreamKey GetInputStreamKeyByPlayer(PlayerKey SessionLevelPlayerID)
		{
			return 0;
		};

		InputStreamKey GetInputStreamKeyByLocalActorKey(ActorKey LocalLevelActorID)
		{
			return 0;
		};

	protected:
		volatile uint64_t highestInput = 0; // volatile is utterly useless for its intended purpose. 
		UCanonicalInputStreamECS* ECSParent;
		
		//Add can only be used by the Artillery Worker Thread through the methods of the UCISArty.
		void add(INNNNCOMING shell, long SentAt)
		{
			CurrentHistory[highestInput].MyInputActions = shell;
			CurrentHistory[highestInput].ReachedArtilleryAt = ECSParent->Now();
			CurrentHistory[highestInput].SentAt = SentAt;//this is gonna get weird after a couple refactors, but that's why we hide it here.

			// reading, adding one, and storing are all separate ops. a slice here is never dangerous but can be erroneous.
			// because this is a volatile variable, it cannot be optimized away and most compilers will not reorder it.
			// however, volatile is basically useless normally. it doesn't provoke a memory fence, and for a variety of reasons
			// it's not suitable for most driver applications. However.
			// 
			// There's a special case which is a monotonically increasing value that is only ever
			// incremented by one thread with a single call site for the increment. In this case, you can still get
			// interleaved but the value will always be either k or k+1. If it's stale in cache, the worst case
			// is that the newest input won't be legible yet and this can be resolved by repolling.
			++highestInput;
		};

		//Overload for local add via feed from cabling. don't use this unless you are CERTAIN.
		void add(INNNNCOMING shell)
		{
			CurrentHistory[highestInput].MyInputActions = shell;
			CurrentHistory[highestInput].ReachedArtilleryAt = ECSParent->Now();
			CurrentHistory[highestInput].SentAt = ECSParent->Now();
			++highestInput;
		};
	};

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

public:
private:
	std::unordered_map < InputStreamKey, TSharedPtr<FConservedInputStream>>* InternalMapping;
	std::unordered_map <PlayerKey, InputStreamKey>* SessionPlayerToStreamMapping;
	std::unordered_map <ActorKey, InputStreamKey>* LocalActorToStreamMapping;
	UBristleconeWorldSubsystem* MySquire;

};


typedef UCanonicalInputStreamECS UCISArty;
typedef UCISArty::FConservedInputStream ArtilleryControlStream;
typedef ArtilleryControlStream FAControlStream;