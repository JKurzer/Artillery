// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BarrageDispatch.h"
#include "SkeletonTypes.h"
#include "KeyCarry.h"
#include "FBarragePrimitive.h"
#include "Components/ActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "BarragePlayerAgent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UBarragePlayerAgent : public UActorComponent
{
	GENERATED_BODY()

public:
	using Caps = 
	UE::Geometry::FCapsule3d;
	// Sets default values for this component's properties
	UPROPERTY()
	double radius;
	UPROPERTY()
	double extent;
	UPROPERTY()
	double taper;
	UBarragePlayerAgent();
	UBarragePlayerAgent(const FObjectInitializer& ObjectInitializer);
	FBLet MyBarrageBody = nullptr;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	ObjectKey MyObjectKey;
	bool IsReady = false;
	virtual void BeforeBeginPlay(ObjectKey TransformOwner);
	void Register();

	virtual void OnDestroyPhysicsState() override;
	// Called when the game starts
	virtual void BeginPlay() override;

	

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};

//CONSTRUCTORS
//--------------------
//do not invoke the default constructor unless you have a really good plan. in general, let UE initialize your components.
inline UBarragePlayerAgent::UBarragePlayerAgent()
{
	PrimaryComponentTick.bCanEverTick = true;
	MyObjectKey = 0;
}
// Sets default values for this component's properties
inline UBarragePlayerAgent::UBarragePlayerAgent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	
	PrimaryComponentTick.bCanEverTick = true;
	MyObjectKey = 0;
	
}
//---------------------------------

//SETTER: Unused example of how you might set up a registration for an arbitrary key.
inline void UBarragePlayerAgent::BeforeBeginPlay(ObjectKey TransformOwner)
{
	MyObjectKey = TransformOwner;
}

//KEY REGISTER, initializer, and failover.
//----------------------------------

inline void UBarragePlayerAgent::Register()
{
	if(MyObjectKey ==0 )
	{
		if(GetOwner())
		{
			if(GetOwner()->GetComponentByClass<UKeyCarry>())
			{
				MyObjectKey = GetOwner()->GetComponentByClass<UKeyCarry>()->GetObjectKey();
			}

			if(MyObjectKey == 0)
			{
				auto val = PointerHash(GetOwner());
				ActorKey TopLevelActorKey = ActorKey(val);
				MyObjectKey = TopLevelActorKey;
			}
		}
	}
	if(!IsReady && MyObjectKey != 0) // this could easily be just the !=, but it's better to have the whole idiom in the example
	{
		auto Physics =  GetWorld()->GetSubsystem<UBarrageDispatch>();
		auto TransformECS =  GetWorld()->GetSubsystem<UTransformDispatch>();
		if(Cap)
		{
			auto params = FBarrageBounder::GenerateCharacterBounds(TransformECS->GetKineByObjectKey(MyObjectKey)->CopyOfTransformLike()->GetLocation(), radius, extent, taper);
			MyBarrageBody = Physics->CreatePrimitive(params, MyObjectKey, LayersMap::MOVING);
			//TransformECS->RegisterObjectToShadowTransform(MyObjectKey, const_cast<UE::Math::TTransform<double>*>(&GetOwner()->GetTransform()));
			if(MyBarrageBody)
			{
				IsReady = true;
			}
		}
	}
	if(IsReady)
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
	}
}


inline void UBarragePlayerAgent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Register();// ...
}

// Called when the game starts
inline void UBarragePlayerAgent::BeginPlay()
{
	Super::BeginPlay();
	Register();
}

//TOMBSTONERS

inline void UBarragePlayerAgent::OnDestroyPhysicsState()
{
	Super::OnDestroyPhysicsState();
	if(GetWorld())
	{
		auto Physics =  GetWorld()->GetSubsystem<UBarrageDispatch>();
		if(Physics && MyBarrageBody)
		{
			Physics->SuggestTombstone(MyBarrageBody);
			MyBarrageBody.Reset();
		}
	}
}

inline void UBarragePlayerAgent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if(GetWorld())
	{
		auto Physics =  GetWorld()->GetSubsystem<UBarrageDispatch>();
		if(Physics && MyBarrageBody)
		{
			Physics->SuggestTombstone(MyBarrageBody);
			MyBarrageBody.Reset();
		}
	}
}
