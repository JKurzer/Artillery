
#pragma once
#include "Ticklite.h"
#include "ArtilleryDispatch.h"
#include "FArtilleryTicklitesThread.h"

	//A ticklite's impl component(s) must provide:
	//TICKLITE_StateReset on the memory block aspect
	//TICKLITE_Calculate on the impl aspect
	//TICKLITE_Apply on the impl aspect, consuming the memory block aspect's state
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
			ObjectKey Target
			) : TL_ThreadedImpl(), RechargeTarget(Target)
		{
		}
		void TICKLITE_StateReset()
		{
		}
		void TICKLITE_Calculate()
		{
		}


		void RechargeClamp(AttrPtr bindH, AttribKey Max, AttribKey Current)
		{
			if(bindH != nullptr && bindH->GetCurrentValue() > 0)
			{
				auto bindHMax = TL_ThreadedImpl::ADispatch->GetAttrib(RechargeTarget, Max);
				auto bindHCur = TL_ThreadedImpl::ADispatch->GetAttrib(RechargeTarget, Current);
				if(
					(bindHMax != nullptr && bindHMax->GetCurrentValue() > 0) &&
					(bindHCur != nullptr)) //note that current does not check 0. lmao. it used to.
				{
					auto clamped = std::min(bindH->GetCurrentValue() + bindHCur->GetCurrentValue(), bindHMax->GetCurrentValue());
					bindHCur->SetCurrentValue(clamped);
				}
			}
		}

		//This can be set up to autowire, but I'm not sure we're keeping these mechanisms yet.
		//we can speed this up considerably by adding a get all attribs. not sure we wanna though until optimization demands it.
		void TICKLITE_Apply()
		{
			//factor the get attr down to the impl.
			auto ManaRecharge =  TL_ThreadedImpl::ADispatch->GetAttrib(RechargeTarget,  Attr::ManaRechargePerTick);
			auto ShieldRecharge =  TL_ThreadedImpl::ADispatch->GetAttrib(RechargeTarget, Attr::ShieldsRechargePerTick);
			auto HealthRecharge =  TL_ThreadedImpl::ADispatch->GetAttrib(RechargeTarget, Attr::HealthRechargePerTick);
			
			RechargeClamp(HealthRecharge, Attr::MaxHealth, Attr::Health);
			RechargeClamp(ShieldRecharge, Attr::MaxShields, Attr::Shields);
			RechargeClamp(ManaRecharge, Attr::MaxMana, Attr::Mana);
		}
		
		void TICKLITE_CoreReset()
		{
		}

		bool TICKLITE_CheckForExpiration()
		{
			return false; //add check for aliveness of ya owner, factor that down.
		}

		void TICKLITE_OnExpiration()
		{
			//no op
		}
	};

typedef Ticklites::Ticklite<TLRecharger> Recharger;

