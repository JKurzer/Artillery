#pragma once

#include "Ticklite.h"
#include "ArtilleryDispatch.h"
#include "FArtilleryTicklitesThread.h"
#include "FWorldSimOwner.h"

class FTSphereCast : public UArtilleryDispatch::TL_ThreadedImpl {
private:
	uint32 TicksRemaining;
	FSkeletonKey ShapeCastSourceObject;
	float Radius;
	float Distance;
	FVector RayStart;
	FVector RayDirection;
	FBarrageKey SphereIntoBarrageKey;
	
public:
	FTSphereCast() : TicksRemaining(20), ShapeCastSourceObject(0), Radius(20), Distance(20), SphereIntoBarrageKey(0) {
	}
	
	FTSphereCast(
		FSkeletonKey ShapeCastSource,
		float SphereRadius,
		float CastDistance,
		const FVector& StartLocation,
		const FVector& Direction,
		FBarrageKey IntoBarrageKey)
	: TicksRemaining(20),
	ShapeCastSourceObject(ShapeCastSource),
	Radius(SphereRadius), Distance(CastDistance),
	RayStart(StartLocation),
	RayDirection(Direction),
	SphereIntoBarrageKey(IntoBarrageKey) {
	}
	
	void TICKLITE_StateReset() {
	}
	
	void TICKLITE_Calculate() {
		UBarrageDispatch* Physics = this->ADispatch->DispatchOwner->GetWorld()->GetSubsystem<UBarrageDispatch>();
		if (Physics) {
			FBLet* HitObject = Physics->SphereCast(SphereIntoBarrageKey, Radius, Distance, RayStart, RayDirection);
			if (HitObject) {
				FSkeletonKey ObjectKey = HitObject->Get()->KeyOutOfBarrage;
				AttrPtr HitObjectHealthPtr = this->ADispatch->GetAttrib(ObjectKey, HEALTH);
				if (HitObjectHealthPtr.IsValid()) {
					float HitObjectHealthVal = HitObjectHealthPtr->GetCurrentValue();
					UE_LOG(LogTemp, Warning, TEXT("Hit Object Health = '%f'"), HitObjectHealthVal);
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