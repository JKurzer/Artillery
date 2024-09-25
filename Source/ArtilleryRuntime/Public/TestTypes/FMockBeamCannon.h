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
struct ARTILLERYRUNTIME_API FMockBeamCannon : public FArtilleryGun {
	GENERATED_BODY()
	
public:
	friend class UArtilleryPerActorAbilityMinimum;
	UArtilleryDispatch* MyDispatch;

	// Gun parameters
	float Range;

	FMockBeamCannon(const FGunKey& KeyFromDispatch, float BeamGunRange) {
		UE_LOG(LogTemp, Warning, TEXT("FMockBeamCannon constructor"));
		MyDispatch = nullptr;
		MyGunKey = KeyFromDispatch;
		Range = BeamGunRange;
	};
	
	FMockBeamCannon() {
		MyDispatch = nullptr;
		MyGunKey = Default;
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
			UArtilleryPerActorAbilityMinimum* FFC = nullptr) override {
		UE_LOG(LogTemp, Warning, TEXT("FMockBeamCannon initialize"));
		MyDispatch = GWorld->GetSubsystem<UArtilleryDispatch>();
		
		return ARTGUN_MACROAUTOINIT(MyCodeWillHandleKeys);
	}

	virtual void PreFireGun(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData = nullptr,
		bool RerunDueToReconcile = false,
		int DallyFramesToOmit = 0) override {
		FireGun(FArtilleryStates::Fired, 0, ActorInfo, ActivationInfo, false, TriggerEventData , Handle);
	};

	virtual void FireGun(
		FArtilleryStates OutcomeStates,
		int DallyFramesToOmit,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool RerunDueToReconcile,
		const FGameplayEventData* TriggerEventData,
		FGameplayAbilitySpecHandle Handle) override {
		if (ActorInfo->OwnerActor.IsValid()) {
			if (UCameraComponent* CameraComponent = ActorInfo->OwnerActor->GetComponentByClass<UCameraComponent>())
			{
				FVector StartLocation = CameraComponent->GetComponentLocation() + FVector(-10.0f, 0.0f, 0.0f);
				//FVector StartLocation = ActorInfo->OwnerActor->GetActorLocation();// + FVector(-10.0f, 0.0f, 100.0f);
				FRotator Rotation = CameraComponent->GetRelativeRotation();
				//ActorInfo->OwnerActor->GetActorEyesViewPoint(StartLocation, Rotation);
			
			const FVector TraceEnd = StartLocation + Rotation.Vector() * 20000.0f;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(ActorInfo->OwnerActor.Get());
			
			FHitResult Hit;
			MyDispatch->GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, TraceEnd, ECC_Camera, QueryParams);
			DrawDebugLine(MyDispatch->GetWorld(), StartLocation, TraceEnd, FColor::Blue, false, 5.0f, 0, 10.0f);

			UBarrageDispatch* Physics = MyDispatch->GetWorld()->GetSubsystem<UBarrageDispatch>();
			FBLet OwnerFiblet = Physics->GetShapeRef(MyProbableOwner);
			FTSphereCast temp = FTSphereCast(OwnerFiblet->KeyIntoBarrage, 0.01f, Range, StartLocation,Rotation.Vector());
			MyDispatch->RequestAddTicklite(
				MakeShareable(new TL_SphereCast(temp)), Early);
	
				PostFireGun(FArtilleryStates::Fired, 0, ActorInfo, ActivationInfo, false, TriggerEventData, Handle);
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
						FGameplayAbilitySpecHandle Handle) override {
	};

private:
	static const inline FGunKey Default = FGunKey("Laser", UINT64_MAX);
};
