```c++
//provided as a suitable base to cut down on cloney-rigatoni
//replace MemoryBlock* as appropriate
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
class FTLinearVelocity /*Facaded*/
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
	void TICKLITE_Apply(MemoryBlock* SelfRef)
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
typedef Ticklites::Ticklite<FTLinearVelocity, FArtilleryTicklitesWorker*> TL_LinearVelocity;

```
