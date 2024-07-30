// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "UBristleconeWorldSubsystem.h"
#include "UCablingWorldSubsystem.h"
#include "ArtilleryCommonTypes.h"
#include "Containers/TripleBuffer.h"
#include "FArtilleryBusyWorker.h"
#include "FArtilleryGun.h"
#include "LocomotionParams.h"
#include "ConservedAttribute.h"
#include "FArtilleryTicklitesThread.h"
#include "ArtilleryDispatch.generated.h"


/**
 * This is the backbone of artillery, and along with the ArtilleryGuns and UFireControlMachines, 
 * this is most of what any other code will see of artillery in C++. Blueprints will mostly not even see that
 * as they are already encapsulated in the artillerygun's ability sequencer. Abilities merely need to be uninstanced,
 * only modify attributes tracked in Artillery, and use artillery bp nodes where possible.
 * 
 * No unusual additional serialization or similar is required for determinism. I hope.
 * 
 * For an organized view of the code, start by understanding that most work is done outside the gamethread.
 * Bristlecone handles network inputs, Cabling handles keymapping and local inputs. Artillery handles outcomes.
 * 
 * Here's the core rule:
 * 
 * Core gameplay simulation happens in the ArtilleryBusyWorker at about 120hz.
 * Anything that cannot cause the core gameplay sim to become desyncrhonized happens in the game thread.
 * Surprisingly, this means _most things flow through UE normally and happen on the game thread & render threads._
 * 
 * Our goal is that cosmetic ability components, animation, IK, particle systems, cloth, and really, almost everything
 * happens in UE normally. Rollbacks don't happen in the broader UE system at all, and in fact, most of UE acts like its
 * running in a pretty simple Iris push config.
 * 
 * Artillery just pushes transform and ability updates upward as needed. That's a big just, but it really is a narrow scope.
 * 
 * Iris does normal replication on a slow cadence as a fall back and to provide attribute sync reassurances.
 */
namespace Arty
{
	typedef TSharedPtr<FConservedAttributeData> AttrPtr;
	typedef TMap<AttribKey, AttrPtr> AttributeMap;
	typedef TSharedPtr<AttributeMap> AttrMapPtr;
	typedef TPair<FTransform3d*, FTransform3d> RealAndShadowTransform;
	DECLARE_DELEGATE_TwoParams(FArtilleryFireGunFromDispatch,
		TSharedPtr<FArtilleryGun> Gun,
		bool InputAlreadyUsedOnce);
	//returns true if-and-only-if the duration of the input intent was exhausted.
	DECLARE_DELEGATE_RetVal_FourParams(bool,
		FArtilleryRunLocomotionFromDispatch,
		FArtilleryShell PreviousMovement,
		FArtilleryShell Movement,
		bool RunAtLeastOnce,
		bool Smear);

}

class UCanonicalInputStreamECS;
UCLASS()
class ARTILLERYRUNTIME_API UArtilleryDispatch : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	friend class FArtilleryBusyWorker;
	friend class UCanonicalInputStreamECS;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	//this is the underlying function mapping we use to queue up Gun Activations.
	//These will eventually need a complete and consistent ordering to ensure determinism.
	//copy op is intentional but may be unneeded. assess before revising signature.
	//TODO: assess if this needs to be a multimap. I think it needs to NOT be.
	TSharedPtr< TMap<FGunKey, FArtilleryFireGunFromDispatch>> GunToFiringFunctionMapping;
	TSharedPtr<BufferedEvents> RequestorQueue_Abilities_TripleBuffer;

	//This is more straightforward than the guns problem.
	//We can actually map this quite directly.
	TSharedPtr< TMap<ActorKey, FArtilleryRunLocomotionFromDispatch>> ActorToLocomotionMapping;

	//We can't touch the uobjects, but the ftransforms are simply PODs.
	//by modifying a parent transform rather than the actual transform, we can avoid a data contention.
	//by using a gamesimT parent over the gamedisplayT, we can actually do this. Jesus christ. for now,
	//we hack it, but...... this works. this is threadsafe. monstrous, but threadsafe after a fashion.
	//by god.
	//this likely needs to be a write-safe conc data structure for true speed.
	//I'm considering GrowOnlyLockFreeHash.h
	//temporarily, I'm just locking and prayin', prayin and lockin'.
	//todo: add proper shadowing either with a conserved transform (OUGH) or something clever. good luck.
	TSharedPtr< TMap<ObjectKey, RealAndShadowTransform>> ObjectToTransformMapping;
	//todo, build FAttributeSet. :/
	TSharedPtr<TMap<ObjectKey, AttrMapPtr>> AttributeSetToDataMapping;


	FTransform3d&  GetTransformShadowByObjectKey(ObjectKey Target, ArtilleryTime Now);
	//this is about as safe as eating live hornets right now.
	
	//todo: convert conserved attribute to use a timestamp for versioning to create a true temporal shadowstack.
	//todo: swap the fuck to FAttributeSet after building it. :/
	AttrMapPtr GetAttribSetShadowByObjectKey(
		ObjectKey Target, ArtilleryTime Now) const;

	TSharedPtr<BufferedMoveEvents> RequestorQueue_Locomos_TripleBuffer;

	static inline long long TotalFirings = 0; //2024 was rough.
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//fully specifying the type is necessary to prevent spurious warnings in some cases.
	TSharedPtr<TCircularQueue<std::pair<FGunKey, Arty::ArtilleryTime>>> ActionsToOrder;
	//These two are the backbone of the Artillery gun lifecycle.
	TSharedPtr< TMap<FGunKey, TSharedPtr<FArtilleryGun>>> GunByKey;
	TMultiMap<FString, TSharedPtr<FArtilleryGun>> PooledGuns;

	
	/**
	 * Wil hold the configuration for the gun definitions
	 */
	TObjectPtr<UDataTable> GunDefinitionsManifest;
	//TODO: This is a dummy function and should be replaced NLT 10/20/24.
	//It loads from the PROJECT directory. This cannot ship, but will work for all purposes at the moment.
	//Note: https://www.reddit.com/r/unrealengine/comments/160mjkx/how_reliable_and_scalable_are_the_data_tables/
	void LoadGunData();
	
	TSharedPtr<TCircularQueue<std::pair<FGunKey, Arty::ArtilleryTime>>> ActionsToReconcile;

	void QueueFire(FGunKey Key, Arty::ArtilleryTime Time);

	void QueueResim(FGunKey Key, Arty::ArtilleryTime Time);

	//the separation of tick and frame is inspired by the Serious Engine and others.
	//In fact, it's pretty common to this day, with Unity also using a similar model.
	//However, our particular design is running fast relative to most games except quake.
	void RunGuns();
	void RunLocomotions();
	void RunRecharges();
	void RunGunFireTimers();
	void CheckFutures();
	//The current start of the tick boundary that ticklites should run on. this allows the ticklites
	//to run in frozen time.
	inline ArtilleryTime GetShadowNow()
	const
	{
		return ArtilleryAsyncWorldSim.TickliteNow;
	};
	void HACK_RunVelocityStack();
	//********************************
	//DUMMY. USED BY RECONCILE AND RERUN.
	//Here to demonstrate that we actually queue normal and rerun separately.
	//We can't risk intermingling them, which should never happen, but...
	//c'mon. Seriously. you wanna find that bug?
	//********************************
	void RERunGuns();
	void RERunLocomotions();

public:
	//DUMMY FOR NOW.
	//TODO: IMPLEMENT THE GUNMAP FROM INSTANCE UNTO CLASS
	//TODO: REMEMBER TO SAY AMMO A BUNCH
	void ApplyShadowTransforms();
	FGunKey GetGun(FString GunDefinitionID, FireControlKey MachineKey);
	FGunKey RegisterExistingGun(FArtilleryGun* toBind, ActorKey ProbableOwner) const;
	bool ReleaseGun(FGunKey Key, FireControlKey MachineKey);

	AttrPtr GetAttrib(ActorKey Owner, AttribKey Attrib);
	
	void RegisterReady(FGunKey Key, FArtilleryFireGunFromDispatch Machine)
	{
		GunToFiringFunctionMapping->Add(Key, Machine);
	}
	void RegisterLocomotion(ActorKey Key, FArtilleryRunLocomotionFromDispatch Machine)
	{
		ActorToLocomotionMapping->Add(Key, Machine);
	}
	void Deregister(FGunKey Key)
	{
		GunToFiringFunctionMapping->Remove(Key);
		//TODO: add the rest of the wipe here?
	}
	void RegisterAttributes(ObjectKey in, AttrMapPtr Attributes)
	{
		AttributeSetToDataMapping->Add(in, Attributes);
	}
	
	//THIS CREATES A COPY FOR THE SHADOW BUT UPDATES THE SHADOW EVERY TICK.
	//THIS IS NOT CHEAP.
	void RegisterObjectToShadowTransform(ObjectKey Target, FTransform3d* Original);
	std::atomic_bool UseNetworkInput;
	bool missedPrior = false;
	bool burstDropDetected = false;

private:
	static inline long long monotonkey = 0;
	//If you're trying to figure out how artillery works, read the busy worker knowing it's a single thread coming off of Dispatch.
	//this handles input from bristlecone, patching it into streams from the CanonicalInputStreamECS (ACIS), using the ACIS to perform mappings,
	//and processing those streams using the pattern matcher. right now, we also expect it to own rollback and jolt when that's implemented.
	//
	// 
	// This is quite a lot and for performance or legibility reasons there is a good chance that this thread will
	//need to be split up. On the other hand, we want each loop iteration to run for around 2-4 milliseconds.
	//this is quite a bit.
	FArtilleryBusyWorker ArtilleryAsyncWorldSim;
	//This runs the tickables alongside the worldsim, ticking each time the worldsim ticks
	//but NOT using locks. So the processing is totally independent. This is extremely fast,
	//extremely powerful, and the reason why we don't use ticklites when we don't need to.
	//it's dangerous as __________ _____________________ _ _________.
	
	FArtilleryTicklitesWorker<UArtilleryDispatch> ArtilleryTicklitesWorker_LockstepToWorldSim;
	TUniquePtr<FRunnableThread> WorldSim_Thread;
	TUniquePtr<FRunnableThread> WorldSim_Ticklites_Thread;
	FSharedEventRef StartTicklitesSim;
	FSharedEventRef StartTicklitesApply;
};
