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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Movement, meta=(ClampMin="0", UIMin="0"))
	float TurningBoost = 1.1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Movement)
	float MaxVelocity = 600;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Movement)
	float Deceleration = 200;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Movement)
	float Acceleration = 200;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Movement)
	float AirAcceleration = 7;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Movement)
	float JumpImpulse = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Movement)
	float WallJumpImpulse = 500;



	[[nodiscard]] FVector Chaos_LastGameFrameRightVector() const
	{
		return CHAOS_LastGameFrameRightVector.IsNearlyZero() ? FVector::RightVector : CHAOS_LastGameFrameRightVector;
	}

	[[nodiscard]] FVector Chaos_LastGameFrameForwardVector() const
	{
		return CHAOS_LastGameFrameForwardVector.IsNearlyZero() ? FVector::ForwardVector : CHAOS_LastGameFrameForwardVector ;
	}

	UBarragePlayerAgent(const FObjectInitializer& ObjectInitializer);
	virtual void Register() override;
	void AddForce(float Duration);
	void ApplyRotation(float Duration, FQuat4f Rotation);
	void AddOneTickOfForce(FVector3d Force);
	// Kludge for now until we double-ify everything
	void AddOneTickOfForce(FVector3f Force);
	FVector3f GetVelocity();
	FVector3f GetGroundNormal();
	bool IsOnGround();
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ApplyAimFriction(const ActorKey& ActorsKey, const FVector3d& ActorLocation, const FVector3d& Direction, FVector2d& OutAimVector);
	
protected:
	UPROPERTY(BlueprintReadOnly)
	FVector CHAOS_LastGameFrameRightVector = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly)
	FVector CHAOS_LastGameFrameForwardVector = FVector::ZeroVector;

private:
	// Currently targeted object
	FBLet TargetFiblet;
	TWeakObjectPtr<AActor> TargetPtr;
};

//CONSTRUCTORS
//--------------------
//do not invoke the default constructor unless you have a really good plan. in general, let UE initialize your components.

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

inline FVector3f UBarragePlayerAgent::GetGroundNormal()
{
	return FBarragePrimitive::GetCharacterGroundNormal(MyBarrageBody);
}

inline bool UBarragePlayerAgent::IsOnGround()
{
	return FBarragePrimitive::IsCharacterOnGround(MyBarrageBody);
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
	if(!IsReady && MyObjectKey != 0 && !GetOwner()->GetActorLocation().ContainsNaN()) // this could easily be just the !=, but it's better to have the whole idiom in the example
	{
		auto Physics =  GetWorld()->GetSubsystem<UBarrageDispatch>();
			auto params = FBarrageBounder::GenerateCharacterBounds(GetOwner()->GetActorLocation(), radius, extent, MaxVelocity);
			MyBarrageBody = Physics->CreatePrimitive(params, MyObjectKey, Layers::MOVING);

			if(MyBarrageBody)
			{
				IsReady = true;
			}
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

inline void UBarragePlayerAgent::AddOneTickOfForce(FVector3f Force)
{
	FBarragePrimitive::ApplyForce(FVector3d(Force.X, Force.Y, Force.Z), MyBarrageBody);
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
	if(!IsReady)
	{
		Register();// ...
	}

	CHAOS_LastGameFrameRightVector = GetOwner()->GetActorRightVector();
	CHAOS_LastGameFrameForwardVector = GetOwner()->GetActorForwardVector();
	
}

inline void UBarragePlayerAgent::ApplyAimFriction(
	const ActorKey& ActorsKey,
	const FVector3d& ActorLocation,
	const FVector3d& Direction,
	FVector2d& OutAimVector)
{
	double TotalFriction = 1.0f;
	
	UBarrageDispatch* Physics = GetWorld()->GetSubsystem<UBarrageDispatch>();
	check(Physics);
	
	FBLet MyFiblet = Physics->GetShapeRef(ActorsKey);
	check(MyFiblet); // The actor calling this sure as hell better be allocated already

	TSharedPtr<FHitResult> HitObjectResult = MakeShared<FHitResult>();
	Physics->SphereCast(
		MyFiblet->KeyIntoBarrage,
		0.01f,
		1000.0f, // Hard-coding range for now until we determine how we want to handle range on this
		ActorLocation,
		Direction,
		HitObjectResult);

	FBarrageKey HitBarrageKey = Physics->GetBarrageKeyFromFHitResult(HitObjectResult);

	// Determine if we've changed targets
	if (HitBarrageKey != 0)
	{
		if (!TargetFiblet.IsValid() || HitBarrageKey != TargetFiblet->KeyIntoBarrage)
		{
			TargetFiblet = Physics->GetShapeRef(HitBarrageKey);
			check(TargetFiblet.IsValid());
		
			UTransformDispatch* TransformDispatch = GetWorld()->GetSubsystem<UTransformDispatch>();
			check(TransformDispatch);

			FBLet PotentialTargetFiblet = Physics->GetShapeRef(HitBarrageKey);
			TWeakObjectPtr<AActor> PotentialNewTarget = TransformDispatch->GetAActorByObjectKey(TargetFiblet->KeyOutOfBarrage);
			if (PotentialNewTarget.IsValid() && PotentialNewTarget.Get()->Tags.Contains(FName("enemy")))
			{
				TargetPtr = PotentialNewTarget;
			}
		}
	}
	else
	{
		TargetFiblet.Reset();
		TargetPtr.Reset();
	}

	if (TargetPtr.IsValid())
	{
		// TODO - determine if aim vector is moving towards or away from a friction point
		TotalFriction = 0.5f;
	}

	UE_LOG(LogTemp, Warning, TEXT("Target found, applying friction to reticle ('%f')"), TotalFriction);
	OutAimVector *= TotalFriction;
}
