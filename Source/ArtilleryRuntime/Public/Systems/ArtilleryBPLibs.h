
#include "ArtilleryDispatch.h"
#include "CanonicalInputStreamECS.h"
#include "PhysicsTypes/BarragePlayerAgent.h"

UCLASS(meta=(ScriptName="InputSystemLibrary"))
class ARTILLERYRUNTIME_API UInputECSLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, meta = (ScriptName = "Get15PlayerInputs", DisplayName = "Get Last 15 of Local Player's Inputs", WorldContext = "WorldContextObject", HidePin = "WorldContextObject"),  Category="Artillery|Inputs")
	static void K2_Get15LocalHistoricalInputs(UObject* WorldContextObject, TArray<FArtilleryShell> &Inputs)
	{
		auto ptr = WorldContextObject->GetWorld()->GetSubsystem<UCanonicalInputStreamECS>();
		if(ptr)
		{
			auto streamkey = ptr->GetStreamForPlayer(PlayerKey::CABLE);
			auto sptr = ptr->GetStream(streamkey);
			for(int i = 0; i <= 15; ++i)
			{
				auto input =  sptr.Get()->peek( sptr->GetHighestGuaranteedInput());
				Inputs.Add(input.has_value() ? input.value() : FArtilleryShell());
			}
		}
	}

};

UCLASS(meta=(ScriptName="AbilitySystemLibrary"))
class ARTILLERYRUNTIME_API UArtilleryLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, meta = (ScriptName = "GetAttribute", DisplayName = "Get Attribute Of", ExpandBoolAsExecs="bFound"), Category="Artillery|Attributes")
	static float K2_GetAttrib(FSkeletonKey Owner, E_AttribKey Attrib, bool& bFound)
	{
		bFound = false;
		return implK2_GetAttrib(Owner,Attrib, bFound);
	}

	static float implK2_GetAttrib(FSkeletonKey Owner, E_AttribKey Attrib, bool& bFound)
	{
		
		bFound = false;
		if(UArtilleryDispatch::SelfPtr)
		{
			if(UArtilleryDispatch::SelfPtr->GetAttrib( Owner, Attrib))
			{
				bFound = true;
				return UArtilleryDispatch::SelfPtr->GetAttrib( Owner, Attrib)->GetCurrentValue();
			}
		}
		return NAN;
	}
	UFUNCTION(BlueprintCallable, meta = (ScriptName = "GetRelatedKey", DisplayName = "Get Related Key From", ExpandBoolAsExecs="bFound"), Category="Artillery|Keys")
	static FSkeletonKey K2_GetIdentity(FSkeletonKey Owner, E_IdentityAttrib Attrib, bool& bFound)
	{
		
		bFound = false;
		return implK2_GetIdentity(Owner, Attrib, bFound);
	}
	
	static FSkeletonKey implK2_GetIdentity(FSkeletonKey Owner, E_IdentityAttrib Attrib, bool& bFound)
	{
		
		bFound = false;
		if(UArtilleryDispatch::SelfPtr)
		{
			auto ident = UArtilleryDispatch::SelfPtr->GetIdent( Owner, Attrib);
			if(ident)
			{
				bFound = true;
				return ident->CurrentValue;
			}
		}
		return FSkeletonKey();
	}

	UFUNCTION(BlueprintCallable, meta = (ScriptName = "GetPlayerRelatedKey", DisplayName = "Get Local Player's Related Key", WorldContext = "WorldContextObject", HidePin = "WorldContextObject", ExpandBoolAsExecs="bFound"),  Category="Artillery|Keys")
	static FSkeletonKey K2_GetPlayerIdentity(UObject* WorldContextObject, E_IdentityAttrib Attrib, bool& bFound)
	{
		
		bFound = false;
		auto ptr = WorldContextObject->GetWorld()->GetSubsystem<UCanonicalInputStreamECS>();
		if(ptr)
		{
			auto streamkey = ptr->GetStreamForPlayer(PlayerKey::CABLE);
			auto key = ptr->ActorByStream(streamkey);
			if(key)
			{
				return  implK2_GetIdentity(key, Attrib, bFound);
			}
		}
		bFound = false;
		return FSkeletonKey();
	}

	UFUNCTION(BlueprintCallable, meta = (ScriptName = "GetThisActorAttribute", DisplayName = "Get My Actor's Attribute", DefaultToSelf = "Actor", HidePin = "Actor", ExpandBoolAsExecs="bFound"),  Category="Artillery|Attributes")
	static float K2_GetMyAttrib(AActor *Actor, E_AttribKey Attrib, bool& bFound)
	{
	
		bFound = false;
		auto ptr = Actor->GetComponentByClass<UKeyCarry>();
		if(ptr)
		{
			if(FSkeletonKey key = ptr->GetObjectKey())
			{
				return implK2_GetAttrib(key, Attrib, bFound);
			}
		}
		bFound = false;
		return NAN;
	}

	UFUNCTION(BlueprintCallable, meta = (ScriptName = "GetPlayerAttribute", DisplayName = "Get Local Player's Attribute", WorldContext = "WorldContextObject", HidePin = "WorldContextObject", ExpandBoolAsExecs="bFound"),  Category="Artillery|Attributes")
	static float K2_GetPlayerAttrib(UObject* WorldContextObject, E_AttribKey Attrib, bool& bFound)
	{
		bFound = false;
		auto ptr = WorldContextObject->GetWorld()->GetSubsystem<UCanonicalInputStreamECS>();
		if(ptr)
		{
			auto streamkey = ptr->GetStreamForPlayer(PlayerKey::CABLE);
			auto key = ptr->ActorByStream(streamkey);
			if(key)
			{
				return  implK2_GetAttrib(key, Attrib, bFound);
			}
		}
		bFound = false;
		return NAN;
	}
	//DEPRECATED
	//TODO: This needs to be replaced by GetPlayerBarrageAgent(PlayerKey)
	static TObjectPtr<UBarragePlayerAgent> GetLocalPlayerBarrageAgent()
	{
		if(CurrentPlayerAgent && CurrentPlayerAgent->GetOwner() && CurrentPlayerAgent->GetOwner()->IsActorTickEnabled())
		{
			return CurrentPlayerAgent;
		}
		else if(UTransformDispatch::SelfPtr && UArtilleryDispatch::SelfPtr && UCanonicalInputStreamECS::SelfPtr)
		{
			CurrentPlayerAgent = nullptr;
			auto local = UCanonicalInputStreamECS::SelfPtr->GetStreamForPlayer(PlayerKey::CABLE);
			if(local != 0)
			{
				auto playerkey = UCanonicalInputStreamECS::SelfPtr->ActorByStream(local);
				TObjectPtr<AActor> SecretName =  UTransformDispatch::SelfPtr->GetAActorByObjectKey(playerkey).Get();
				if(SecretName)
				{
					CurrentPlayerAgent = SecretName->GetComponentByClass<UBarragePlayerAgent>();
					return CurrentPlayerAgent;
				}
			}
		}
		return nullptr;
	}

	//DEPRECATED
	//TODO: This needs to be replaced by GetPlayerVectors(Forward, Right, PlayerKey)
	static void GetLocalPlayerVectors(FVector& Forward, FVector& Right)
	{
		if(UTransformDispatch::SelfPtr && UArtilleryDispatch::SelfPtr && UCanonicalInputStreamECS::SelfPtr)
		{
			auto local = GetLocalPlayerBarrageAgent();
			if(local && local->IsValidLowLevelFast())
			{
				Forward = local->Chaos_LastGameFrameForwardVector();
				Right = local->Chaos_LastGameFrameRightVector();
			}
		}
	}

	UFUNCTION(BlueprintPure, meta = (ScriptName = "GetPlayerVectors", DisplayName = "Get Local Player's Attribute"),  Category="Artillery|Character")
	static void K2_GetLocalPlayerVectors(FVector& Forward, FVector& Right)
	{
		GetLocalPlayerVectors(Forward, Right);
	}

	UFUNCTION(BlueprintPure, meta = (ScriptName = "GetPlayerDirectionEstimator", DisplayName = "Get Local Player's Attribute"),  Category="Artillery|Character")
	static void K2_GetPlayerDirectionEstimator(FVector& Forward)
	{
		FVector Right;
		GetLocalPlayerVectors(Forward, Right);
		
	}

	
private:
	static inline TObjectPtr<UBarragePlayerAgent> CurrentPlayerAgent = nullptr;
};