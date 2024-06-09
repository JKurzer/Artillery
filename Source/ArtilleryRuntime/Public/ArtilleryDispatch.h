// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ArtilleryCommonTypes.h"
#include "FArtilleryBusyWorker.h"
#include "ArtilleryDispatch.generated.h"

/**
 * Component for separating UI dependencies.
 */
UCLASS()
class ARTILLERYRUNTIME_API UArtilleryDispatch : public UTickableWorldSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

public:
	bool registerPattern(TSharedPtr<FActionPattern> ToBind, FActionBitMask ToSeek, FGunKey ToFire);
	bool removePattern(TSharedPtr<FActionPattern> ToBind, FActionBitMask ToSeek, FGunKey ToFire);
	FGunKey getNewGunInstance(FString GunDefinitionID);

private:
	FArtilleryBusyWorker ArtilleryAsyncWorldSim;

	TUniquePtr<FRunnableThread> WorldSim_Thread;
};
