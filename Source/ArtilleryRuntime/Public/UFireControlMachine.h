// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreTypes.h"
#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include <unordered_map>

#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "FArtilleryGun.h"
#include "Abilities/GameplayAbility.h"

#include "Containers/CircularQueue.h"
#include "CanonicalInputStreamECS.h"
#include <bitset>
#include "ArtilleryDispatch.h"
#include "ArtilleryCommonTypes.h"
#include "Components/ActorComponent.h"
#include "UFireControlMachine.generated.h"

//dynamic constructed statemachine for matching patterns in action records to triggering abilities.




//Generally, a firecontrolmachine represents a set of guns that share attributes (like ammo!)
// 
//Even then, this attribute set will most often be empty, as we generally want to manage the attributes
//on the actor itself. this will mean we need to think carefully about how to expose the correct attrib set
//down to the blueprint of the abilities themselves.

//The fire control machine is basically a pointer to the abilities an actor has, and manages their activation patterns.
//It's pretty simple, with most of the implementation actually running on the ability system component or the abilities.
//it does not interact directly with input or run the patterns itself.
// 
//Patterns are always run by the artillery worker thread. events generated by pattern success will likely
//run on the tick of either this component or its parent actor for now, but may come to run async if we get lucky.
// 
// Like "Guns," this class mostly exists to hide Artillery complexity from people familiar with GAS already.
// Combined with the ACS, allows you to fully integrate artillery with your existing GAS designs.
UCLASS()
class ARTILLERYRUNTIME_API UFireControlMachine : public UActorComponent

{

	GENERATED_BODY()

public:	
		UArtilleryDispatch* MySquire;
		UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TMap<FGunKey, FArtilleryGun> MyManagedGuns;
		//this needs to be replicated in iris, interestin'ly.
		TObjectPtr <UAttributeSet> MyAttributes; // might want to defactor this to an ECS, but I actually quite like it here.
		//still wondering who owns the input streams...
		TObjectPtr<UAbilitySystemComponent> SystemComponentToBind;
		
		//Should this include the input stream key? probably?
		bool pushPatternToRunner(TSharedPtr<FActionPattern> ToBind, FActionBitMask ToSeek, FGunKey ToFire)
		{
			return MySquire->registerPattern(ToBind, ToSeek, ToFire);
		};

		bool popPatternFromRunner(TSharedPtr<FActionPattern> ToBind, FActionBitMask ToSeek, FGunKey ToFire)
		{
			return MySquire->removePattern(ToBind, ToSeek, ToFire);
		};


		bool FireGun(FGunKey GunID, bool InputAlreadyUsedOnce)
		{
			return false;
		};
		
		void BeginPlay() override {
			Super::BeginPlay();
			MySquire = GetOwner()->GetWorld()->GetSubsystem<UArtilleryDispatch>();
			//likely want to manage system component bind here by checking for actor parent.
			//right now, we can push all our patterns here as well, and we can use a static set of patterns for
			//each of our fire control machines. you can basically think of a fire control machine as a full set
			//of related abilities, their attributes, and similar required to, you know, actually fire a gun.
			//There's a bit more blueprint exposure work to do here as a result.
		};
};