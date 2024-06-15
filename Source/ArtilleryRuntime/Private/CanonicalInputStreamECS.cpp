// Fill out your copyright notice in the Description page of Project Settings.


#include "CanonicalInputStreamECS.h"

void UCanonicalInputStreamECS::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("Artillery::CanonicalInputStream is Online."));
}

void UCanonicalInputStreamECS::OnWorldBeginPlay(UWorld& InWorld)
{
	if ([[maybe_unused]] const UWorld* World = InWorld.GetWorld()) {
		UE_LOG(LogTemp, Warning, TEXT("Artillery::CanonicalInputStream is Operational"));
		MySquire = GetWorld()->GetSubsystem<UBristleconeWorldSubsystem>();
		}
}

void UCanonicalInputStreamECS::Deinitialize()
{
	UE_LOG(LogTemp, Warning, TEXT("Artillery::CanonicalInputStream is Shutting Down."));
	Super::Deinitialize();
}

void UCanonicalInputStreamECS::Tick(float DeltaTime)
{
}

TStatId UCanonicalInputStreamECS::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCanonicalInputStreamECS, STATGROUP_Tickables);
}




bool UCanonicalInputStreamECS::registerPattern(TSharedPtr<FActionPattern_InternallyStateless> ToBind,
	FActionPatternParams FCM_Owner_ActorParams)
{
	if (
#ifndef LOCALISCODEDSPECIAL
		FCM_Owner_ActorParams.MyInputStream == 0xb33f ||
#endif // !LOCALISCODEDSPECIAL
		InternalMapping.Contains(FCM_Owner_ActorParams.MyInputStream))
	{
		auto thisInputStream = InternalMapping.Find(FCM_Owner_ActorParams.MyInputStream)->Get();
		if (thisInputStream->MyPatternMatcher->AllPatternBinds.Contains(ToBind->getName()))
		{
			//names are never removed. sets are only added to or removed from.
			thisInputStream->MyPatternMatcher->AllPatternBinds.Find(ToBind->getName())->Get()->Add(FCM_Owner_ActorParams);

		}
		else
		{
			thisInputStream->MyPatternMatcher->Names.Add(ToBind->getName());
			thisInputStream->MyPatternMatcher->AllPatternsByName.Add(ToBind->getName(), ToBind);
			TSharedPtr<TSet<FActionPatternParams>> newSet = MakeShareable < TSet<FActionPatternParams>>(new TSet<FActionPatternParams>());
			newSet.Get()->Add(FCM_Owner_ActorParams);
			thisInputStream->MyPatternMatcher->AllPatternBinds.Add(ToBind->getName(), newSet);
		}

		return true;
	}
	return false;
}

bool UCanonicalInputStreamECS::removePattern(TSharedPtr<FActionPattern_InternallyStateless> ToBind, FActionPatternParams FCM_Owner_ActorParams)
{
	if (
#ifndef LOCALISCODEDSPECIAL
		FCM_Owner_ActorParams.MyInputStream == 0xb33f ||
#endif // !LOCALISCODEDSPECIAL
		InternalMapping.Contains(FCM_Owner_ActorParams.MyInputStream))
	{
		auto thisInputStream = InternalMapping.Find(FCM_Owner_ActorParams.MyInputStream)->Get();
		if (thisInputStream->MyPatternMatcher->AllPatternBinds.Contains(ToBind->getName()))
		{
			//names are never removed. sets are only added to or removed from.
			auto pinSharedPtr = thisInputStream->MyPatternMatcher->AllPatternBinds.Find(ToBind->getName());

			if (pinSharedPtr->Get()->Contains(FCM_Owner_ActorParams))
			{
				auto remId = pinSharedPtr->Get()->FindId(FCM_Owner_ActorParams);
				pinSharedPtr->Get()->Remove(remId);
				return true;
			}
		}
		
	}
	return false;
}
ActorKey UCanonicalInputStreamECS::registerFCMKeyToParentActorMapping(AActor* parent, FireControlKey MachineKey, TObjectKey<UFireControlMachine> MachineSelf)
{
	//todo, registration goes here.
	ActorKey ParentKey = PointerHash(parent, MachineKey);
	LocalActorToStreamMapping.Add(ParentKey, MachineKey);
	return ParentKey;
}
