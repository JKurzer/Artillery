// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BarrageDispatch.h"
#include "CoreTypeKeys.h"
#include "KeyCarry.h"
#include "FBarragePrimitive.h"
#include "Components/ActorComponent.h"
#include "BarrageGravityOnlyTester.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UBarrageGravityOnlyTester : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBarrageGravityOnlyTester();
	FBLet MyBarrageBody;

protected:
	virtual void OnDestroyPhysicsState() override;

public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	ObjectKey MyObjectKey;
	bool IsReady = false;
	virtual void BeginDestroy() override;
	virtual void BeforeBeginPlay(ObjectKey TransformOwner);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};

// Sets default values for this component's properties
inline UBarrageGravityOnlyTester::UBarrageGravityOnlyTester()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.

	MyObjectKey = 0;
	
}

inline void UBarrageGravityOnlyTester::OnDestroyPhysicsState()
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

inline void UBarrageGravityOnlyTester::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

inline void UBarrageGravityOnlyTester::BeginDestroy()
{
	Super::BeginDestroy();

}

inline void UBarrageGravityOnlyTester::BeforeBeginPlay(ObjectKey TransformOwner)
{
	MyObjectKey = TransformOwner;
	IsReady = true;
}


inline void UBarrageGravityOnlyTester::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(MyBarrageBody && FBarragePrimitive::IsNotNull(MyBarrageBody))
	{
		//nothing is required here. weird. real weird.
	}
	// ...
}

// Called when the game starts
inline void UBarrageGravityOnlyTester::BeginPlay()
{
	Super::BeginPlay();
	if(!IsReady)
	{
		MyObjectKey = SKELETON::KeyOf(	this);
		IsReady = true;
	}
	if(MyObjectKey != 0)
	{
		auto Physics =  GetWorld()->GetSubsystem<UBarrageDispatch>();
		auto params = FBarrageBounder::GenerateBoxBounds(GetOwner()->GetActorLocation(), 2, 2 ,2);
		MyBarrageBody = Physics->CreatePrimitive(params, MyObjectKey, LayersMap::MOVING);
	}
	PrimaryComponentTick.SetTickFunctionEnable(false);
}
