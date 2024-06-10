// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
/**
 * Component for managing input streams in an ECS-like way, where any controller can request any stream.
 */


#include "FActionBitMask.h"
#include "BristleconeCommonTypes.h"
typedef TheCone::PacketElement INNNNCOMING;
typedef uint32_t InputStreamKey;
typedef uint32_t PlayerKey;
typedef uint32_t ActorKey;
typedef uint32_t FireControlKey;





#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "AttributeSet.h"
#include <bitset>
#include "Containers/CircularBuffer.h"
#include "FGunKey.h"
struct FActionPatternParams
{
public:
	bool preferToMatch = false;
	bool passThrough = false;
	bool defaultBehavior = false;
	bool FiresCosmetics = false;

	FGunKey ToFire; //Putting this here is likely a mistake.
	FActionBitMask ToSeek;
	InputStreamKey MyInputStream;
	FireControlKey MyOrigin;
	FActionPatternParams(FActionBitMask ToSeek_In, FireControlKey MyOrigin_In, InputStreamKey MyInputStream_In) :
		ToSeek(ToSeek_In), MyOrigin(MyOrigin_In), MyInputStream(MyInputStream_In) {};
};
#include "FActionPattern.h"

