// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FWorldSimOwner.h"
#include "GameFramework/Actor.h"
#include "UEnemyMachine.h"
#include "SwarmKine.h"
#include "AInstancedMeshManager.generated.h"

UCLASS()
class ARTILLERYRUNTIME_API AInstancedMeshManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Artillery, meta = (AllowPrivateAccess = "true"))
	USwarmKineManager* SwarmKineManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Artillery, meta = (AllowPrivateAccess = "true"))
	UArtilleryDispatch* MyDispatch;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Artillery, meta = (AllowPrivateAccess = "true"))
	UTransformDispatch* TransformDispatch;

	virtual void BeginPlay() override
	{
		Super::BeginPlay();

		if(!IsDefaultSubobject())
		{
			MyDispatch = GetWorld()->GetSubsystem<UArtilleryDispatch>();
			TransformDispatch =  GetWorld()->GetSubsystem<UTransformDispatch>();
			// No Chaos for you!
			SwarmKineManager->SetEnableGravity(false);
			SwarmKineManager->SetSimulatePhysics(false);
			SwarmKineManager->DestroyPhysicsState();

			// Make a key yo
			auto keyHash = PointerHash(this);
			UE_LOG(LogTemp, Warning, TEXT("AInstancedMeshManager Parented: %d"), keyHash);
			MyKey = ActorKey(keyHash);
			Usable = true;
		}
	}

public:
	bool Usable = false;

	AInstancedMeshManager(UStaticMesh* MeshToUse)
	{
		SwarmKineManager = CreateDefaultSubobject<USwarmKineManager>("SwarmKineManager");
		SwarmKineManager->SetStaticMesh(MeshToUse);
		MyDispatch = nullptr;
		TransformDispatch = nullptr;
		SkeletonKeyToFBLetMapping = MakeShareable(new TMap<FSkeletonKey, FBLet>());
	}
	
	AInstancedMeshManager()
	{
		SwarmKineManager = CreateDefaultSubobject<USwarmKineManager>("SwarmKineManager");
		MyDispatch = nullptr;
		TransformDispatch = nullptr;
		SkeletonKeyToFBLetMapping = MakeShareable(new TMap<FSkeletonKey, FBLet>());
	}
	ActorKey GetMyKey() const
	{
		return MyKey;
	};

	UFUNCTION(BlueprintCallable, Category = Instance)
	FSkeletonKey CreateNewInstance(const FTransform& WorldTransform, const bool isMovable)
	{
		int32 NewInstanceId = SwarmKineManager->AddInstance(WorldTransform, true);
		// TODO: Does this make a good hash? Can we hash collide?
		auto hash = PointerHash(SwarmKineManager, NewInstanceId);
		FSkeletonKey NewInstanceKey = FSkeletonKey(hash);

		SwarmKineManager->AddToMap(FSMInstanceId(SwarmKineManager, NewInstanceId), NewInstanceKey);

		// TODO: can't use the BarrageColliderBase set of types, so in-lining the barrage setup code. Is this what we want long-term?
		auto Physics =  GetWorld()->GetSubsystem<UBarrageDispatch>();
		auto TransformECS =  GetWorld()->GetSubsystem<UTransformDispatch>();
		auto AnyMesh = SwarmKineManager->GetStaticMesh();
		auto Boxen = AnyMesh->GetBoundingBox();
		auto extents = Boxen.GetExtent() * 2;

		auto params = FBarrageBounder::GenerateBoxBounds(WorldTransform.GetLocation(), extents.X, extents.Y, extents.Z,
			FVector3d(0, 0, extents.Z/2));

		FBLet MyBarrageBody = Physics->CreatePrimitive(params, NewInstanceKey, isMovable ? Layers::MOVING : Layers::NON_MOVING);
		SkeletonKeyToFBLetMapping->Add(NewInstanceKey, MyBarrageBody);

		TransformDispatch->RegisterObjectToShadowTransform(NewInstanceKey, SwarmKineManager);
		
		return NewInstanceKey;
	}

	// THIS MUST BE CALLED OR ELSE THE MAPPINGS WILL KEEP THE LIVE REFERENCE 4EVA

	void CleanupInstance(const FSkeletonKey Target)
	{
		FBLet* MyBarrageBody = SkeletonKeyToFBLetMapping->Find(Target);
		if (MyBarrageBody != nullptr)
		{
			// TODO: IDK Do we need to do stuff here to cleanup the barrage physics object?
		}

		SkeletonKeyToFBLetMapping->Remove(Target);
		SwarmKineManager->CleanupInstance(Target);
	}

private:
	ActorKey MyKey;
	TSharedPtr<TMap<FSkeletonKey, FBLet>> SkeletonKeyToFBLetMapping;
};