#pragma once

#include "Ticklite.h"
#include "ArtilleryDispatch.h"
#include "FArtilleryTicklitesThread.h"

class FTJumpTimer : public UArtilleryDispatch::TL_ThreadedImpl
{
private:
	ObjectKey JumpTarget;

public:
	FTJumpTimer() : TL_ThreadedImpl()
	{
	}

	FTJumpTimer(ObjectKey Jumper) : TL_ThreadedImpl(), JumpTarget(Jumper)
	{
	}

	void TICKLITE_StateReset()
	{
	}

	void TICKLITE_Calculate()
	{
	}
	
	void TICKLITE_Apply() {
		auto ticksLeftPtr = ADispatch->GetAttrib(JumpTarget, Attr::TicksTilJumpAvailable);
		if (!ticksLeftPtr)
		{
			return;
		}

		ticksLeftPtr->SetCurrentValue(ticksLeftPtr->GetCurrentValue() - 1);

		UE_LOG(LogTemp, Display, TEXT("Reducing Jump Ticks"));
	}

	void TICKLITE_CoreReset() {
	}

	bool TICKLITE_CheckForExpiration() {
		auto ticksLeftPtr = ADispatch->GetAttrib(JumpTarget, Attr::TicksTilJumpAvailable);

		// If there is no valid ptr, expire
		if (!ticksLeftPtr)
		{
			return true;
		}

		return ticksLeftPtr->GetCurrentValue() <= 0;
	}

	void TICKLITE_OnExpiration() {
	}
};

typedef Ticklites::Ticklite<FTJumpTimer> TL_JumpTimer;