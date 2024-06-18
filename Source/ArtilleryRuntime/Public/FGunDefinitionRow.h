#pragma once
#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "Engine/DataTable.h"

USTRUCT(BlueprintType)
struct FGunDefinition : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GunDefinition)
	FString GunDefinitionId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GunDefinition)
	FString PreFireAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GunDefinition)
	FString PreFireCosmeticAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GunDefinition)
	FString FireAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GunDefinition)
	FString FireCosmeticAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GunDefinition)
	FString PostFireAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GunDefinition)
	FString PostFireCosmeticAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GunDefinition)
	FString FailureCosmeticAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GunDefinition)
	int32 BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GunDefinition)
	int32 BaseRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GunDefinition)
	int32 BaseRateOfFire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GunDefinition)
	int32 BaseRecoil;

	//0: single fire
	//1: hold fire
	//2: fire on release
	//3: stick flick
	//4: Feathered Hold - Do Not Use Yet.
	//Unsure at this point in implementation if this value will always be respected.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GunDefinition)
	int32 IntendedRegistrationPattern;
};
