#pragma once

#include "Ticklite.h"
#include "ArtilleryDispatch.h"
#include "FArtilleryTicklitesThread.h"
#include "FWorldSimOwner.h"

class FTSphereCast : public UArtilleryDispatch::TL_ThreadedImpl
{
	uint32 TicksRemaining;
	FBarrageKey ShapeCastSourceObject;
	float Radius;
	float Distance;
	FVector RayStart;
	FVector RayDirection;
	TSharedPtr<FHitResult> HitResultPtr;

public:
	FTSphereCast() : TicksRemaining(2), ShapeCastSourceObject(0), Radius(0.01), Distance(5000), HitResultPtr()
	{
	}

	FTSphereCast(
		FBarrageKey ShapeCastSource,
		float SphereRadius,
		float CastDistance,
		const FVector& StartLocation,
		const FVector& Direction,
		const TSharedPtr<FHitResult>& HitResult)
		: TicksRemaining(1),
		  ShapeCastSourceObject(ShapeCastSource),
		  Radius(SphereRadius), Distance(CastDistance),
		  RayStart(StartLocation),
		  RayDirection(Direction),
		  HitResultPtr(HitResult)
	{
	}

	void TICKLITE_StateReset()
	{
	}

	void TICKLITE_Calculate()
	{
		UBarrageDispatch* Physics = this->ADispatch->DispatchOwner->GetWorld()->GetSubsystem<UBarrageDispatch>();
		if (Physics)
		{
			Physics->SphereCast(ShapeCastSourceObject, Radius, Distance, RayStart, RayDirection, HitResultPtr);

			if (HitResultPtr->MyItem != JPH::BodyID::cInvalidBodyID)
			{
				FBarrageKey HitBarrageKey = Physics->GenerateBarrageKeyFromBodyId(
					static_cast<uint32>(HitResultPtr->MyItem));
				FBLet HitObjectFiblet = Physics->GetShapeRef(HitBarrageKey);

				FSkeletonKey ObjectKey = HitObjectFiblet->KeyOutOfBarrage;
				AttrPtr HitObjectHealthPtr = this->ADispatch->GetAttrib(ObjectKey, HEALTH);

				if (HitObjectHealthPtr.IsValid())
				{
					HitObjectHealthPtr->SetCurrentValue(HitObjectHealthPtr->GetCurrentValue() - 5);
				}
			}
		}
	}

	void TICKLITE_Apply()
	{
		--TicksRemaining;
	}

	void TICKLITE_CoreReset()
	{
	}

	bool TICKLITE_CheckForExpiration()
	{
		return TicksRemaining == 0;
	}

	void TICKLITE_OnExpiration()
	{
	}
};

using TL_SphereCast = Ticklites::Ticklite<FTSphereCast>;
