#pragma once
#include "ArtilleryCommonTypes.h"

template<typename ParentThreadAnchor>
struct TL_Impl
{
	//Each class generated gets a unique static. Each kind of dispatcher will get a unique class.
	//TODO: If you run more than one of the parent threads, this gets unsafe. We don't so...
	//As is, it saves a huge amount of memory and indirection costs.
	static ParentThreadAnchor* ADispatch = nullptr;

	explicit TL_Impl(ParentThreadAnchor* Dispatch)
	{
		ADispatch = Dispatch;
	}
	
	ArtilleryTime GetShadowNow()
	{
		return ADispatch->GetShadowNow();
	}
};
