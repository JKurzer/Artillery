// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "AttributeSet.h"
#include "Containers/CircularBuffer.h"
#include "BristleconeCommonTypes.h"

#include "ArtilleryShell.generated.h"
/**
 Artillery provides an opaque storage optimized containerization for the bristlecone packed input, 
 effectively hiding the complexities from the rest of the system. We will need to refactor this to an abstract 
 class and specialize it for inputs, as M+KB starts to matter. That machinery is best suited to living elsewhere,
 as we work towards supporting remap through reuse of EnhancedInput. While we can't use their event loop, 
 we CAN use everything else in EnhancedInput. Let's be careful not to rewrite it.
 */
typedef long BristleTime;//this will become uint32. don't bitbash this.
typedef BristleTime ArtilleryTime;
USTRUCT(BlueprintType)
struct ARTILLERYRUNTIME_API FArtilleryShell
{
	GENERATED_BODY()
public:
	TheCone::PacketElement MyInput;
	BristleTime SentAt;
	ArtilleryTime ReachedArtilleryAt;

private:
	//TODO ADD METHODS FOR GET STICKS, GET BUTTONS, GET EVENTS.
	//DO NOT LET THE BITBASHING OUT OF THE BOX.

};
