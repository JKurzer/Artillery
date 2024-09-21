// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BarrageColliderBase.h"
#include "BarrageDispatch.h"
#include "SkeletonTypes.h"
#include "KeyCarry.h"
#include "FBarragePrimitive.h"
#include "FWorldSimOwner.h"
#include "Components/ActorComponent.h"
#include "BarrageAutoBox.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UBarrageAutoBox : public UBarrageColliderBase
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isMovable = true;
	UBarrageAutoBox();
	UBarrageAutoBox(const FObjectInitializer& ObjectInitializer);
	virtual void Register() override;
};

//CONSTRUCTORS
//--------------------
//do not invoke the default constructor unless you have a really good plan. in general, let UE initialize your components.
inline UBarrageAutoBox::UBarrageAutoBox()
{
	PrimaryComponentTick.bCanEverTick = true;
	MyObjectKey = 0;
}
// Sets default values for this component's properties
inline UBarrageAutoBox::UBarrageAutoBox(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	
	PrimaryComponentTick.bCanEverTick = true;
	MyObjectKey = 0;
	
}


//KEY REGISTER, initializer, and failover.
//----------------------------------

inline void UBarrageAutoBox::Register()
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
	if(!IsReady && MyObjectKey != 0 && GetOwner()) // this could easily be just the !=, but it's better to have the whole idiom in the example
	{
		auto Physics =  GetWorld()->GetSubsystem<UBarrageDispatch>();
		auto TransformECS =  GetWorld()->GetSubsystem<UTransformDispatch>();
		auto Box = GetOwner()->CalculateComponentsBoundingBoxInLocalSpace();
		
		auto extents = Box.GetSize();
		auto params = FBarrageBounder::GenerateBoxBounds(GetOwner()->GetActorLocation(),extents.X , extents.Y ,extents.Z);
		MyBarrageBody = Physics->CreatePrimitive(params, MyObjectKey,  isMovable ? Layers::MOVING : Layers::NON_MOVING);
		//TransformECS->RegisterObjectToShadowTransform(MyObjectKey, const_cast<UE::Math::TTransform<double>*>(&GetOwner()->GetTransform()));
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