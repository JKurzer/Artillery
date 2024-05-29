// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "BristleconeCommonTypes.h"
#include "Templates/SubclassOf.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "AttributeSet.h"
#include "Containers/CircularBuffer.h"
#include "BristleconeCommonTypes.h"
#include "UBristleconeWorldSubsystem.h"
#include "ConservedAttribute.generated.h"
#include <optional>
#include "CanonicalInputStreamECS.generated.h"



/**
 * Component for managing input streams in an ECS-like way, where any controller can request any stream.
 */

typedef TheCone::PacketElement ArtilleryShell; //Do not rely on using this. Factor any use into helpers. It will be hidden soon and only unpacked input will be available.
typedef TheCone::Packet_tpl* INNNNCOMING;
typedef long BristleTime;//this will become uint32. don't bitbash this.
typedef BristleTime ArtilleryTime;
typedef uint32_t InputStreamKey;

UCLASS()
class ARTILLERYRUNTIME_API UCanonicalInputStreamECS : public UTickableWorldSubsystem
{
	GENERATED_BODY()

/**
 * Conserved input streams record their last 8,192 inputs. Yes, that's a few megs. No, it doesn't really seem to matter.
 * Currently, this is for debug purposes, but we can use it with some additional features to provide a really expressive
 * model for rollback at a SUPER granular level if needed. UE has existing rollback tech, though so...
 */
public:
	static ArtilleryTime Now()
	{
		return MySquire->Now();
	};

	class ARTILLERYRUNTIME_API FConservedInputStream
	{
	public:
		TCircularBuffer<ArtilleryShell> CurrentHistory = TCircularBuffer<ArtilleryShell>(8192); //these two should be one buffer of a shared type, but that makes using them harder
		TCircularBuffer<BristleTime> SendStamps = TCircularBuffer<BristleTime>(8192);				//and we also want to have some type insulation here. note that type is stripped.
		TCircularBuffer<uint32_t> ReceiptStamps = TCircularBuffer<uint32_t>(8192); //this could cause a caching perf issue, but I think it'll be fine.
		InputStreamKey MyKey; //in case, god help us, we need a lookup based on this for something else. that should NOT happen.



		void add(INNNNCOMING shells)
		{
			long indexInput = shells->GetCycleMeta() + 3; //faster than 3xabs or a branch.
			CurrentHistory[highestInput] = *(shells->GetPointerToElement(indexInput%3));
			ReceiptStamps[highestInput] = UCISArty::Now();
			SendStamps[highestInput] = shells->GetTransferTime();//this is gonna get weird after a couple refactors, but that's why we hide it here.
			++highestInput;
		};

		std::optional<ArtilleryShell> get(uint64_t input)
		{
			// the highest input is a reserved write-slot.
			// the bound of 8k instead of 8192 is just an old man's superstition, but it should prevent some scrobbles.
			if (input >= highestInput || (highestInput - input) > 8000) 
			{
				return std::optional<ArtilleryShell>(
					std::nullopt
				);
			}
			else {
				return std::optional<ArtilleryShell>(CurrentHistory[input]);
			}
		};

	protected:
		uint64_t highestInput = 0;

	};

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

public:
private:
	static UBristleconeWorldSubsystem* MySquire;

};

typedef UCanonicalInputStreamECS UCISArty;
typedef UCISArty::FConservedInputStream ArtilleryControlStream;
typedef ArtilleryControlStream FAControlStream;
