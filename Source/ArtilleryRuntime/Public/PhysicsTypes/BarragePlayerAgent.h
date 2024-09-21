// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BarrageColliderBase.h"
#include "BarrageDispatch.h"
#include "SkeletonTypes.h"
#include "KeyCarry.h"
#include "FBarragePrimitive.h"
#include "Components/ActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "BarragePlayerAgent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARTILLERYRUNTIME_API UBarragePlayerAgent : public UBarrageColliderBase
{
	GENERATED_BODY()

	//This leans HARD on the collider base but retains more uniqueness than the others.
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Movement, meta=(ClampMin="0", UIMin="0"))
	float TurningBoost = 1.1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Movement)
	float Deceleration = 200;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Movement)
	float Acceleration = 200;
	UBarragePlayerAgent();
	UBarragePlayerAgent(const FObjectInitializer& ObjectInitializer);
	virtual void Register() override;
	void AddForce(float Duration);
	void ApplyRotation(float Duration, FQuat4f Rotation);
	void AddOneTickOfForce(FVector3d Force);
	FVector3f GetVelocity();
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

inline FVector3f UBarragePlayerAgent::GetVelocity()
{
	return FBarragePrimitive::GetVelocity(MyBarrageBody);
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

			auto params = FBarrageBounder::GenerateCharacterBounds(TransformECS->GetKineByObjectKey(MyObjectKey)->CopyOfTransformLike()->GetLocation(), radius, extent, taper);
			MyBarrageBody = Physics->CreatePrimitive(params, MyObjectKey, LayersMap::MOVING);

			if(MyBarrageBody)
			{
				IsReady = true;
			}
	}
	if(IsReady)
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
	}
}

inline void UBarragePlayerAgent::AddForce(float Duration)
{
	//I'll be back for youuuu.
	throw;
}

inline void UBarragePlayerAgent::ApplyRotation(float Duration, FQuat4f Rotation)
{
	//I'll be back for youuuu.
	throw;
}

inline void UBarragePlayerAgent::AddOneTickOfForce(FVector3d Force)
{
	FBarragePrimitive::ApplyForce(Force, MyBarrageBody);
}

// Called when the game starts
inline void UBarragePlayerAgent::BeginPlay()
{
	Super::BeginPlay();
	Register();
}

inline void UBarragePlayerAgent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Register();// ...
}
