#pragma once

#include "Ticklite.h"
#include "ArtilleryDispatch.h"
#include "FArtilleryTicklitesThread.h"
#include "FWorldSimOwner.h"

class FTSphereCast : public UArtilleryDispatch::TL_ThreadedImpl {
private:
	uint32 TicksRemaining;
	ObjectKey ShapeCastSourceObject;
	float Radius;
	float Distance;
	FVector RayStart;
	FVector RayDirection;
	FBarrageKey SphereIntoBarrageKey;
	
public:
	FTSphereCast() : TicksRemaining(20), ShapeCastSourceObject(0), Radius(20), Distance(20), SphereIntoBarrageKey(0) {
	}
	
	FTSphereCast(
		ObjectKey ShapeCastSource,
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
			Physics->SphereCast(SphereIntoBarrageKey, Radius, Distance, RayStart, RayDirection);
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