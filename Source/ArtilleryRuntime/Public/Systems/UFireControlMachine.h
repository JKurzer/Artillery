// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreTypes.h"
#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include <unordered_map>

#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"

#include "GameplayAbilitySpec.h"
#include "GameplayAbilitySpecHandle.h"
#include "FArtilleryGun.h"
#include "Abilities/GameplayAbility.h"
#include "CanonicalInputStreamECS.h"
#include "ArtilleryDispatch.h"
#include <bitset>
#include "ArtilleryCommonTypes.h"
#include "FAttributeMap.h"
#include "FMockArtilleryGun.h"
#include "FMockBeamCannon.h"
#include "FMockDashGun.h"
#include "TransformDispatch.h"
#include "Components/ActorComponent.h"
#include "UFireControlMachine.generated.h"

//dynamic constructed statemachine for matching patterns in action records to triggering abilities.
//extends the Ability System Component to remove even more boiler plate and smoothly integrate
//Artillery's threaded input model.

//The fire control machine manages activation patterns.
//it does not interact directly with input or run the patterns itself.
// 
// Patterns are always run by the artillery worker thread. events generated by pattern success flow through Dispatch.
//
//Each Fire Control Machine manages attributes and abilities, with all abilities being ordered into ArtilleryGuns.
//This additional constraint, combined with the pattern system, allows us to better build abilities that are general
//but NOT abstract. Our goal is to reduce boilerplate and improve ease of use, not necessarily increase code reuse.
//We HOPE that will happen, but bear this set of priorities in mind while dealing with Guns and FireControl.

//As a final word of warning, it has been our extensive experience that generic is better than general, and concrete
//is better than abstract. The urge towards pattern-before-problem programming is deadly.
UCLASS()
class ARTILLERYRUNTIME_API UFireControlMachine : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	static inline int orderInInitialize = 0;
	UCanonicalInputStreamECS* MyInput;
	UArtilleryDispatch* MyDispatch;
	UTransformDispatch* TransformDispatch;
	ActorKey ParentKey;
	//this needs to be replicated in iris, interestin'ly.
	TSet<FGunKey> MyGuns;

	//The direct presence of attributes in this way is likely obsoleted by switching to inheriting rather than friending
	//the UAS component, but I'm not totally sure.
	TSharedPtr<FAttributeMap> MyAttributes;

	FireControlKey MyKey;
	bool Usable = false;

	/** Who to route replication through if ReplicationProxyEnabled (if this returns null, when ReplicationProxyEnabled, we wont replicate)  */
	//this is the tooling used in part by NPP's implementation, and we should consider using it as well to integrate with Iris and constrain
	//replication to attributes only.
	//virtual IAbilitySystemReplicationProxyInterface* GetReplicationInterface();


	// These properties, and the fact that they are NOT the same, is important for understanding the model of the original
	// ability system component. Note that this means by default, this component has no structural model for multi actor
	// groups that might share abilities, such as enemy squads. While this sounds like a corner case, it is not.
	// /** The actor that owns this component logically */
	// UPROPERTY(ReplicatedUsing = OnRep_OwningActor)
	// TObjectPtr<AActor> OwnerActor;
	//
	// /** The actor that is the physical representation used for abilities. Can be NULL */
	// UPROPERTY(ReplicatedUsing = OnRep_OwningActor)
	// TObjectPtr<AActor> AvatarActor;

	/*/** Will be called from GiveAbility or from OnRep. Initializes events (triggers and inputs) with the given ability #1#
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec);

	/** Will be called from RemoveAbility or from OnRep. Unbinds inputs with the given ability #1#
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec);

	/** Called from ClearAbility, ClearAllAbilities or OnRep. Clears any triggers that should no longer exist. #1#
	void CheckForClearedAbilities();

	/** Cancel a specific ability spec #1#
	virtual void CancelAbilitySpec(FGameplayAbilitySpec& Spec, UGameplayAbility* Ignore);

	/** Creates a new instance of an ability, storing it in the spec #1#
	virtual UGameplayAbility* CreateNewInstanceOfAbility(FGameplayAbilitySpec& Spec, const UGameplayAbility* Ability);*/


	/**Found this in the AbilitySystemComponent.h:
	 *
	 *	The abilities we can activate. 
	 *		-This will include CDOs for non instanced abilities and per-execution instanced abilities. 
	 *		-Actor-instanced abilities will be the actual instance (not CDO)
	 *		
     *	This array is not vital for things to work. It is a convenience thing for 'giving abilities to the actor'. But abilities could also work on things
     *	without an AbilitySystemComponent. For example an ability could be written to execute on a StaticMeshActor. As long as the ability doesn't require 
     *	instancing or anything else that the AbilitySystemComponent would provide, then it doesn't need the component to function.
     */

	//it's really not clear how vital granting and removing abilities is. getting my arms around this system is still a lot.
	
	//*******************************************************************************************
	//patterns are run in ArtilleryBusyWorker. Search for ARTILLERY_FIRE_CONTROL_MACHINE_HANDLING
	//*******************************************************************************************

	//IF YOU DO NOT CALL THIS FROM THE GAMETHREAD, YOU WILL HAVE A BAD TIME.
	void pushPatternToRunner(IPM::CanonPattern ToBind, PlayerKey InputStreamByPlayer, FActionBitMask ToSeek, FGunKey ToFire)
	{
		FActionPatternParams myParams = FActionPatternParams(ToSeek, MyKey, InputStreamByPlayer, ToFire);
		MyInput->registerPattern(ToBind, myParams);
		Arty::FArtilleryFireGunFromDispatch Inbound;
		Inbound.BindUObject(this, &UFireControlMachine::FireGun);
		MyDispatch->RegisterReady(ToFire, Inbound);
		MyGuns.Add(ToFire);
	};

	//IF YOU DO NOT CALL THIS FROM THE GAMETHREAD, YOU WILL HAVE A BAD TIME.
	void popPatternFromRunner(FActionPattern* ToBind, PlayerKey InputStreamByPlayer, FActionBitMask ToSeek, FGunKey ToFire)
	{
		FActionPatternParams myParams = FActionPatternParams(ToSeek, MyKey, InputStreamByPlayer, ToFire);
		MyInput->removePattern(ToBind, myParams);
		MyDispatch->Deregister(ToFire);
		MyGuns.Remove(ToFire);


	};


	void AddTestGun(Intents::Intent BindIntent, FArtilleryGun* Gun, IPM::CanonPattern Pattern)
	{
		FActionBitMask IntentBitPattern;
		IntentBitPattern.buttons = BindIntent;
		Gun->Initialize(Gun->MyGunKey, false);
		auto key = MyDispatch->RegisterExistingGun(Gun, ParentKey);
		pushPatternToRunner(Pattern, APlayer::CABLE, IntentBitPattern, key);
	}

	//IF YOU DO NOT CALL THIS FROM THE GAMETHREAD, YOU WILL HAVE A BAD TIME.
	ActorKey CompleteRegistrationByActorParent(bool IsLocalPlayerCharacter,
		FArtilleryRunLocomotionFromDispatch LocomotionFromActor,
		TMap<AttribKey, double> Attributes)
	{
		//these are initialized earlier under all intended orderings, but we cannot ensure that this function will be called correctly
		//so we should do what we can to foolproof things. As long as the world subsystems are up, force-updating
		//here will either:
		//work correctly
		//fail fast
		MyInput = GetWorld()->GetSubsystem<UCanonicalInputStreamECS>();
		MyDispatch = GetWorld()->GetSubsystem<UArtilleryDispatch>();
		TransformDispatch =  GetWorld()->GetSubsystem<UTransformDispatch>();
		TPair<ActorKey, InputStreamKey> Parent =  MyInput->RegisterKeysToParentActorMapping(GetOwner(), MyKey, true);
		ParentKey = Parent.Key;
		MyDispatch->RegisterLocomotion(ParentKey, LocomotionFromActor);
		Usable = true;
		if(MyGuns.IsEmpty() && Usable)
		{
			auto dummy = new FMockArtilleryGun(FGunKey("Dummy", 1));
			AddTestGun(Intents::A, dummy, IPM::GPress);
			auto dash = new FMockDashGun(FGunKey("DummyDash", 2));
			AddTestGun(Intents::B, dash, IPM::GPerPress);
			auto beam = new FMockBeamCannon(FGunKey("DummyBeam", 3), 5000.0f);
			AddTestGun(Intents::RTrigger, beam, IPM::GPerPress);
		}
		MyAttributes = MakeShareable(new FAttributeMap(ParentKey, MyDispatch, Attributes));

		UE_LOG(LogTemp, Warning, TEXT("FCM Mana: %f"), MyDispatch->GetAttrib(ParentKey, Attr::Mana)->GetCurrentValue());

		//DO NOT DO THIS. This is ONLY here until jolt is in place and WILL crash the game.
		TransformDispatch->RegisterObjectToShadowTransform(ParentKey, GetOwner());
		return ParentKey;

		//right now, we can push all our patterns here as well, and we can use a static set of patterns for
		//each of our fire control machines. you can basically think of a fire control machine as a full set
		//of related abilities, their attributes, and similar required to, you know, actually fire a gun.
		//it is also the gas component, if you're using gas.
		//There's a bit more blueprint exposure work to do here as a result.
#ifdef CONTROL_TEST_MODE


#endif
		
	};

	//it is strongly recommended that you understand
	// FGameplayAbilitySpec and FGameplayAbilitySpecDef, as well as Handle.
	// I'm deferring the solve for how we use them for now, in a desperate effort to
	// make sure we can preserve as much of the ability framework as possible
	// but spec management is going to be mission critical for determinism
	void FireGun(TSharedPtr<FArtilleryGun> Gun, bool InputAlreadyUsedOnce)
	{
		//frig.
		FGameplayAbilitySpec BackboneFiring = BuildAbilitySpecFromClass(
			(Gun->Prefire).GetClass(),
			0,
			-1
		);
		FGameplayAbilitySpecHandle FireHandle = BackboneFiring.Handle;
		Gun->PreFireGun(FireHandle,
		AbilityActorInfo.Get(), 
		FGameplayAbilityActivationInfo(EGameplayAbilityActivationMode::Authority)
		);
	};

	void InitializeComponent() override
	{
		Super::InitializeComponent();
		MyKey = UFireControlMachine::orderInInitialize++;
		//we rely on attribute replication, which I think is borderline necessary, but I wonder if we should use effect replication.
		//historically, relying on gameplay effect replication has led to situations where key state was not managed through effects.
		//for OUR situation, where we have few attributes and many effects, huge amounts of effects are likely not interesting for us to replicate.
		ReplicationMode = EGameplayEffectReplicationMode::Minimal; 
	};
	//this happens post init but pre begin play, and the world subsystems should exist by this point.
	//we use this to help ensure that if the actor's begin play triggers first, things will be set correctly
	//I've left the same code in begin play as a fallback.
	void ReadyForReplication() override
	{
		Super::ReadyForReplication();
		MyInput = GetWorld()->GetSubsystem<UCanonicalInputStreamECS>();
		MyDispatch = GetWorld()->GetSubsystem<UArtilleryDispatch>();
	}

	//on components, begin play can fire twice, because we aren't allowed to have nice things.
	//This can cause it to fire BEFORE the actor's begin play fires, which leaves you with
	//very few good options. the bool Usable helps control this.
	//This is, ironically, not a problem in actual usage, only testing, for us.
	void BeginPlay() override
	{
		Super::BeginPlay(); 
		MyInput = GetWorld()->GetSubsystem<UCanonicalInputStreamECS>();
		MyDispatch = GetWorld()->GetSubsystem<UArtilleryDispatch>();
	};
	
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override
	{
		Super::OnComponentDestroyed(bDestroyingHierarchy);
		for (FGunKey Gun : MyGuns)
		{
			MyDispatch->Deregister(Gun); // emergency deregister.
		}
	};
};
