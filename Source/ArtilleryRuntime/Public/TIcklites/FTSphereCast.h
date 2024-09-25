#pragma once

#include "Ticklite.h"
#include "ArtilleryDispatch.h"
#include "FArtilleryTicklitesThread.h"
#include "FWorldSimOwner.h"

class FTSphereCast : public UArtilleryDispatch::TL_ThreadedImpl {
private:
	uint32 TicksRemaining;
	FBarrageKey ShapeCastSourceObject;
	float Radius;
	float Distance;
	FVector RayStart;
	FVector RayDirection;
	
public:
	FTSphereCast() : TicksRemaining(1), ShapeCastSourceObject(0), Radius(0.01), Distance(5000), SphereIntoBarrageKey(0) {
	}
	
	FTSphereCast(
		FBarrageKey ShapeCastSource,
		float SphereRadius,
		float CastDistance,
		const FVector& StartLocation,
		const FVector& Direction)
	: TicksRemaining(1),
	ShapeCastSourceObject(ShapeCastSource),
	Radius(SphereRadius), Distance(CastDistance),
	RayStart(StartLocation),
	RayDirection(Direction) {
	}
	
	void TICKLITE_StateReset() {
	}
	
	void TICKLITE_Calculate() {
		UBarrageDispatch* Physics = this->ADispatch->DispatchOwner->GetWorld()->GetSubsystem<UBarrageDispatch>();
		if (Physics) {
			FBLet* HitObject = Physics->SphereCast(ShapeCastSourceObject, Radius, Distance, RayStart, RayDirection);
			if (HitObject) {
				FSkeletonKey ObjectKey = HitObject->Get()->KeyOutOfBarrage;
				AttrPtr HitObjectHealthPtr = this->ADispatch->GetAttrib(ObjectKey, HEALTH);
				if (HitObjectHealthPtr.IsValid()) {
					float HitObjectHealthVal = HitObjectHealthPtr->GetCurrentValue();
					UE_LOG(LogTemp, Warning, TEXT("Hit Object Health = '%f'"), HitObjectHealthVal);
					HitObjectHealthPtr->SetCurrentValue(HitObjectHealthPtr->GetCurrentValue() - 5);
				} else {
					UE_LOG(LogTemp, Warning, TEXT("Could not get object health"));
				}
			}
		}
	}
	
	void TICKLITE_Apply() {
		--TicksRemaining;
	}
	void TICKLITE_CoreReset() {
	}

	bool TICKLITE_CheckForExpiration() {
		return TicksRemaining == 0;
	}

	void TICKLITE_OnExpiration() {
	}
};

typedef Ticklites::Ticklite<FTSphereCast> TL_SphereCast;