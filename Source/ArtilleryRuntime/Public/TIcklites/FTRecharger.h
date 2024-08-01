
#pragma once
#include "Ticklite.h"
#include "ArtilleryDispatch.h"
#include "FArtilleryTicklitesThread.h"



	//A ticklite's impl component(s) must provide:
	//TICKLITE_StateReset on the memory block aspect
	//TICKLITE_Calculate on the impl aspect
	//TICKLITE_Apply(MemoryBlock*) on the impl aspect, consuming the memory block aspect's state
	//TICKLITE_CoreReset on the impl aspect
	//TICKLITE_CheckForExpiration on the impl aspect
	//TICKLITE_OnExpiration

	class TLRecharger : public UArtilleryDispatch::TL_ThreadedImpl /*Facaded*/
	{
	public:
		ObjectKey RechargeTarget;
		TLRecharger(): TL_ThreadedImpl()
		{
		}

		TLRecharger(
			ObjectKey Target,
			) : RechargeTarget(Target)
		{
		}
		void TICKLITE_StateReset()
		{
		}
		void TICKLITE_Calculate()
		{
		}
		//this isn't quite right. we should calculate the component in calculate
		//but for now, this is good enough for testing.
		void TICKLITE_Apply()
		{

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
			//no op
		}
	};
//behold!
// FTLinearVelocity<FArtilleryTicklitesWorker*>& would allow you to use a reference indirection here, I believe.
typedef Ticklites::Ticklite<TLRecharger> Recharger;