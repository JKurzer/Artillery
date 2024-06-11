// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/UnrealType.h"
#include "Engine/DataTable.h"
#include "AttributeSet.h"
#include "ArtilleryShell.h"
#include "ArtilleryCommonTypes.h"
#include <bitset>
#include "Containers/CircularBuffer.h"
#include <string>

//See Desperate-thor.gif for more information.
//Catch you in the next one, choom.

//This facade offers no guarantee that it is read only. it exists to break a circular dependency in an elegant
//way that also serves to cleanly facade away some quite considerabile complexity.
//it's probably read only. probably.
class FArtilleryNoGuaranteeReadOnly
{
public:
	virtual std::optional<FArtilleryShell> peek(uint64_t input) = 0;
};
//See Desperate-thor.gif for more information or FArtilleryNoGuaranteeReadOnly
typedef TSharedPtr<FArtilleryNoGuaranteeReadOnly> FANG_PTR;