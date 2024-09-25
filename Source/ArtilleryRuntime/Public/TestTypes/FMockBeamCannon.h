#pragma once

#include "CoreMinimal.h"
#include "FArtilleryGun.h"
#include "FTSphereCast.h"
#include "UArtilleryAbilityMinimum.h"
#include "Camera/CameraComponent.h"

#include "FMockBeamCannon.generated.h"

#define SCREEN_MESSAGE(str) if (GEngine) \
		{ \
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, FString::Printf(TEXT(str))); \
		}

/**
* Well THIS test class encapsulates a beam cannon!
* 
*/
USTRUCT(BlueprintType)
struct ARTILLERYRUNTIME_API FMockBeamCannon : public FArtilleryGun
{
	GENERATED_BODY()

	friend class UArtilleryPerActorAbilityMinimum;
	UArtilleryDispatch* MyDispatch;
	FHitResult HitResult;

	// Gun parameters
	float Range;

	FMockBeamCannon(const FGunKey& KeyFromDispatch, float BeamGunRange)
	{
		MyDispatch = nullptr;
		MyGunKey = KeyFromDispatch;
		Range = BeamGunRange;
		HitResult.Init();
	};

	FMockBeamCannon()
	{
		MyDispatch = nullptr;
		MyGunKey = Default;
		HitResult.Init();
		Range = 5000.0f;
	};

	virtual bool Initialize(
		const FGunKey& KeyFromDispatch,
		bool MyCodeWillHandleKeys,
		UArtilleryPerActorAbilityMinimum* PF = nullptr,
		UArtilleryPerActorAbilityMinimum* PFC = nullptr,
		UArtilleryPerActorAbilityMinimum* F = nullptr,
		UArtilleryPerActorAbilityMinimum* FC = nullptr,
		UArtilleryPerActorAbilityMinimum* PtF = nullptr,
		UArtilleryPerActorAbilityMinimum* PtFc = nullptr,
		UArtilleryPerActorAbilityMinimum* FFC = nullptr) override
	{
		MyDispatch = GWorld->GetSubsystem<UArtilleryDispatch>();
		return ARTGUN_MACROAUTOINIT(MyCodeWillHandleKeys);
	}

	virtual void PreFireGun(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData = nullptr,
		bool RerunDueToReconcile = false,
		int DallyFramesToOmit = 0) override
	{
		FireGun(Fired, 0, ActorInfo, ActivationInfo, false, TriggerEventData, Handle);
	};

	virtual void FireGun(
		FArtilleryStates OutcomeStates,
		int DallyFramesToOmit,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool RerunDueToReconcile,
		const FGameplayEventData* TriggerEventData,
		FGameplayAbilitySpecHandle Handle) override
	{
		if (ActorInfo->OwnerActor.IsValid())
		{
			if (UCameraComponent* CameraComponent = ActorInfo->OwnerActor->GetComponentByClass<UCameraComponent>())
			{
				FVector StartLocation = CameraComponent->GetComponentLocation() + FVector(-10.0f, 0.0f, 0.0f);
				FRotator Rotation = CameraComponent->GetRelativeRotation();

				UBarrageDispatch* Physics = MyDispatch->GetWorld()->GetSubsystem<UBarrageDispatch>();
				FBLet OwnerFiblet = Physics->GetShapeRef(MyProbableOwner);

				FTSphereCast temp = FTSphereCast(
					OwnerFiblet->KeyIntoBarrage,
					0.01f,
					Range,
					StartLocation,
					Rotation.Vector(),
					MakeShareable<FHitResult>(&HitResult));
				MyDispatch->RequestAddTicklite(MakeShareable(new TL_SphereCast(temp)), Early);

				if (HitResult.MyItem != JPH::BodyID::cInvalidBodyID)
				{
					DrawDebugLine(
						MyDispatch->GetWorld(),
						StartLocation,
						HitResult.Location,
						FColor::Blue,
						false,
						5.0f,
						0,
						1.0f);
				}
				
				PostFireGun(Fired, 0, ActorInfo, ActivationInfo, false, TriggerEventData, Handle);
			}
		}
	}

	virtual void PostFireGun(
		FArtilleryStates OutcomeStates,
		int DallyFramesToOmit,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool RerunDueToReconcile,
		const FGameplayEventData* TriggerEventData,
		FGameplayAbilitySpecHandle Handle) override
	{
	};

private:
	static const inline FGunKey Default = FGunKey("Laser", UINT64_MAX);
};
