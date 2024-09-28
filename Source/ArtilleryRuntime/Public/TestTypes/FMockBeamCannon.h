#pragma once

#include "CoreMinimal.h"
#include "FArtilleryGun.h"
#include "FTSphereCast.h"
#include "UArtilleryAbilityMinimum.h"
#include "Camera/CameraComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

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

public:
	friend class UArtilleryPerActorAbilityMinimum;
	UArtilleryDispatch* MyDispatch;
	
	// Gun parameters
	float Range;
	
	// Beam effect
	UPROPERTY()
	UNiagaraSystem* Beam;

	FMockBeamCannon(const FGunKey& KeyFromDispatch, int MaxAmmoIn, int FirerateIn, int ReloadTimeIn, float BeamGunRange)
	{
		MyGunKey = KeyFromDispatch;
		MaxAmmo = MaxAmmoIn;
		Firerate = FirerateIn;
		ReloadTime = ReloadTimeIn;
		MyDispatch = nullptr;
		
		Range = BeamGunRange;
		Beam = nullptr;
	};
	
	FMockBeamCannon() : Super()
	{
		MyDispatch = nullptr;
		
		MyGunKey = Default;
		MaxAmmo = 100;
		Firerate = 2;
		ReloadTime = 150;
		Range = 5000.0f;
		Beam = nullptr;
	};

	virtual bool Initialize(
		const FGunKey& KeyFromDispatch,
		const bool MyCodeWillHandleKeys,
		UArtilleryPerActorAbilityMinimum* PF = nullptr,
		UArtilleryPerActorAbilityMinimum* PFC = nullptr,
		UArtilleryPerActorAbilityMinimum* F = nullptr,
		UArtilleryPerActorAbilityMinimum* FC = nullptr,
		UArtilleryPerActorAbilityMinimum* PtF = nullptr,
		UArtilleryPerActorAbilityMinimum* PtFc = nullptr,
		UArtilleryPerActorAbilityMinimum* FFC = nullptr) override
	{
		MyDispatch = GWorld->GetSubsystem<UArtilleryDispatch>();

		Beam = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Blueprints/BeamCannon/BeamSystem.BeamSystem"), nullptr, LOAD_None, nullptr);
		check(Beam != nullptr);
		
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
		AttrPtr CooldownRemainingPtr = MyDispatch->GetAttrib(MyGunKey, COOLDOWN_REMAINING);
		AttrPtr AmmoRemainingPtr = MyDispatch->GetAttrib(MyGunKey, AMMO);
		if (!CooldownRemainingPtr.IsValid() || CooldownRemainingPtr->GetCurrentValue() > 0.f)
		{
			// Cooldown not up yet!
			return;
		}

		if (!AmmoRemainingPtr.IsValid() || AmmoRemainingPtr->GetCurrentValue() <= 0.f)
		{
			// No ammo!
			return;
		}
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

				// Fire particles
				FVector FireParticleLocation = CameraComponent->GetComponentLocation() + FVector(-10.f, 0.f, 0.f);
				UNiagaraComponent* BeamComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(MyDispatch->GetWorld(), Beam, FireParticleLocation, FRotator::ZeroRotator, FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
				BeamComp->SetVariablePosition(FName("Beam_End"), Rotation.Vector() * Range);

				// UE_LOG(LogTemp, Warning, TEXT("Target for particle is '%s'"), *(Rotation.Vector() * Range).ToString());
				
				PostFireGun(Fired, 0, ActorInfo, ActivationInfo, false, TriggerEventData, Handle);
			}
		}
	}

	void ResolveHit(UE::Math::TVector<double> RayStart, TSharedPtr<FHitResult> HitResultFromTicklite) const
	{
		// DrawDebugLine(
		// 				MyDispatch->GetWorld(),
		// 				RayStart,
		// 				HitResultFromTicklite->Location,
		// 				FColor::Blue,
		// 				false,
		// 				5.0f,
		// 				0,
		// 				1.0f);

		UBarrageDispatch* Physics = MyDispatch->GetWorld()->GetSubsystem<UBarrageDispatch>();
		FBarrageKey HitBarrageKey = Physics->GenerateBarrageKeyFromBodyId(
					static_cast<uint32>(HitResultFromTicklite->MyItem));
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
		AttrPtr AmmoPtr = MyDispatch->GetAttrib(MyGunKey, AMMO);
		if (AmmoPtr.IsValid())
		{
			AmmoPtr->SetCurrentValue(AmmoPtr->GetCurrentValue() - 1);
		}
		AttrPtr CooldownPtr = MyDispatch->GetAttrib(MyGunKey, COOLDOWN);
		AttrPtr CooldownRemainingPtr = MyDispatch->GetAttrib(MyGunKey, COOLDOWN_REMAINING);
		if (CooldownPtr.IsValid() && CooldownRemainingPtr.IsValid())
		{
			CooldownRemainingPtr->SetCurrentValue(CooldownPtr->GetCurrentValue());
		}
		
		MyDispatch->GetAttrib(MyGunKey, TICKS_SINCE_GUN_LAST_FIRED)->SetCurrentValue(0.f);
		MyDispatch->GetAttrib(MyGunKey, AttribKey::LastFiredTimestamp)->SetCurrentValue(static_cast<double>(MyDispatch->GetShadowNow()));
	};

private:
	static const inline FGunKey Default = FGunKey("Laser", UINT64_MAX);
};
