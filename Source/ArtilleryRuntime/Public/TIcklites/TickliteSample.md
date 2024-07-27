```c++
#pragma once
#include "Ticklite.h"
#include "ArtilleryDispatch.h"
#include "FArtilleryTicklitesThread.h"
#include "TL_Impl.h"


//A ticklite's impl component(s) must provide:
//TICKLITE_StateReset on the memory block aspect
//TICKLITE_Calculate on the impl aspect
//TICKLITE_Apply(MemoryBlock*) on the impl aspect, consuming the memory block aspect's state
//TICKLITE_CoreReset on the impl aspect
//TICKLITE_CheckForExpiration on the impl aspect
//TICKLITE_OnExpiration
template<typename ParentThreadAnchor>
class FTLinearVelocity : public TL_Impl<ParentThreadAnchor> /*Facaded*/
{
public:
ObjectKey VelocityTarget;
VelocityVec VelocityToApply;
void TICKLITE_StateReset()
{

	}
	void TICKLITE_Calculate()
	{
		
	}
	void TICKLITE_Apply(FTLinearVelocity* SelfRef)
	{
		
	}
	void TICKLITE_CoreReset()
	{
		
	}
	
	void TICKLITE_CheckForExpiration()
	{
		
	}

	TICKLITE_OnExpiration()
	{
		
	}
};
//behold!
typedef Ticklites::Ticklite<FTLinearVelocity<FArtilleryTicklitesWorker*>> TL_LinearVelocity;
```
