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

	// Gun parameters
	float Range;

	FMockBeamCannon(const FGunKey& KeyFromDispatch, float BeamGunRange)
	{
		MyGunKey = KeyFromDispatch;
		Range = BeamGunRange;
	};

	FMockBeamCannon()
	{
		MyGunKey = Default;
		Range = 5000.0f;
	};

	virtual bool Initialize(
		const FGunKey& KeyFromDispatch,
		const TMap<AttribKey, double> Attributes,
		const bool MyCodeWillHandleKeys,
		UArtilleryPerActorAbilityMinimum* PF = nullptr,
		UArtilleryPerActorAbilityMinimum* PFC = nullptr,
		UArtilleryPerActorAbilityMinimum* F = nullptr,
		UArtilleryPerActorAbilityMinimum* FC = nullptr,
		UArtilleryPerActorAbilityMinimum* PtF = nullptr,
		UArtilleryPerActorAbilityMinimum* PtFc = nullptr,
		UArtilleryPerActorAbilityMinimum* FFC = nullptr) override
	{
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
					// TODO: This possibly horribly fails when trying to synchronize network state
					// Not sure what pattern we want to enforce for hit reg callbacks though, so this temporarily works
					std::bind(&FMockBeamCannon::ResolveHit, this, std::placeholders::_1, std::placeholders::_2));
				MyDispatch->RequestAddTicklite(MakeShareable(new TL_SphereCast(temp)), Early);
				
				PostFireGun(Fired, 0, ActorInfo, ActivationInfo, false, TriggerEventData, Handle);
			}
		}
	}

	void ResolveHit(UE::Math::TVector<double> RayStart, TSharedPtr<FHitResult> HitResult)
	{
		DrawDebugLine(
						MyDispatch->GetWorld(),
						RayStart,
						HitResult->Location,
						FColor::Blue,
						false,
						5.0f,
						0,
						1.0f);

		UBarrageDispatch* Physics = MyDispatch->GetWorld()->GetSubsystem<UBarrageDispatch>();
		FBarrageKey HitBarrageKey = Physics->GenerateBarrageKeyFromBodyId(
					static_cast<uint32>(HitResult->MyItem));
		FBLet HitObjectFiblet = Physics->GetShapeRef(HitBarrageKey);

		FSkeletonKey ObjectKey = HitObjectFiblet->KeyOutOfBarrage;
		AttrPtr HitObjectHealthPtr = MyDispatch->GetAttrib(ObjectKey, HEALTH);

		if (HitObjectHealthPtr.IsValid())
		{
			HitObjectHealthPtr->SetCurrentValue(HitObjectHealthPtr->GetCurrentValue() - 5);
		}

	};

	virtual void PostFireGun(
		FArtilleryStates OutcomeStates,
		int DallyFramesToOmit,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool RerunDueToReconcile,
		const FGameplayEventData* TriggerEventData,
		FGameplayAbilitySpecHandle Handle) override
	{
		// TODO: revisit ammo, this is just proof of concept
		AttrPtr AmmoPtr = MyDispatch->GetAttrib(MyGunKey, AMMO);
		AmmoPtr->SetCurrentValue(AmmoPtr->GetCurrentValue() - 1);
		UE_LOG(LogTemp, Warning, TEXT("Remaining Ammo %f"), AmmoPtr->GetCurrentValue());
	};

private:
	static const inline FGunKey Default = FGunKey("Laser", UINT64_MAX);
};
