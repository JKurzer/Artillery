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

ActorKey UCanonicalInputStreamECS::ActorByStream(InputStreamKey Stream)
{
	return StreamToActorMapping->FindRef(Stream);
}

InputStreamKey UCanonicalInputStreamECS::StreamByActor(ActorKey Actor)
{
	return ActorToStreamMapping->FindRef(Actor);
}

void UCanonicalInputStreamECS::Tick(float DeltaTime)
{
}

TStatId UCanonicalInputStreamECS::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCanonicalInputStreamECS, STATGROUP_Tickables);
}

TSharedPtr<UCanonicalInputStreamECS::FConservedInputStream> UCanonicalInputStreamECS::getNewStreamConstruct( PlayerKey ByPlayerConcept)
{
	
	TSharedPtr<ArtilleryControlStream> ManagedStream = MakeShareable(
	new FConservedInputStream(this, ByPlayerConcept) //using++ vs ++would be wrong here. inc then ret.
	);
	auto BifurcateOwnership = new TSharedPtr<ArtilleryControlStream>(ManagedStream);
	SessionPlayerToStreamMapping->Add(ByPlayerConcept, ByPlayerConcept);//this will need, ultimately, to be revised. but for now, it's ordered, and consistent.
	StreamKeyToStreamMapping->Add(ByPlayerConcept, *BifurcateOwnership);//this is a sin.
	return ManagedStream; 
}


InputStreamKey UCanonicalInputStreamECS::GetStreamForPlayer(PlayerKey ThisPlayer)
{
	return SessionPlayerToStreamMapping->FindChecked(ThisPlayer);
}

TSharedPtr<UCanonicalInputStreamECS::FConservedInputStream> UCanonicalInputStreamECS::GetStream(InputStreamKey StreamKey) const
{
	const auto SP = StreamKeyToStreamMapping->FindRef(StreamKey);
	return SP; // creates a copy.
}

bool UCanonicalInputStreamECS::registerPattern( IPM::CanonPattern ToBind,
                                               FActionPatternParams FCM_Owner_ActorParams)
{
	if (
#ifndef LOCALISCODEDSPECIAL
		FCM_Owner_ActorParams.MyInputStream == APlayer::CABLE ||
#endif // !LOCALISCODEDSPECIAL
		StreamKeyToStreamMapping->Contains(FCM_Owner_ActorParams.MyInputStream))
	{
		auto thisInputStream = StreamKeyToStreamMapping->Find(FCM_Owner_ActorParams.MyInputStream)->Get();
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

bool UCanonicalInputStreamECS::removePattern(IPM::CanonPattern ToBind, FActionPatternParams FCM_Owner_ActorParams)
{
	if (
#ifndef LOCALISCODEDSPECIAL
		FCM_Owner_ActorParams.MyInputStream == APlayer::CABLE ||
#endif // !LOCALISCODEDSPECIAL
		StreamKeyToStreamMapping->Contains(FCM_Owner_ActorParams.MyInputStream))
	{
		auto thisInputStream = StreamKeyToStreamMapping->Find(FCM_Owner_ActorParams.MyInputStream)->Get();
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
TPair<ActorKey, InputStreamKey> UCanonicalInputStreamECS::RegisterKeysToParentActorMapping(AActor* parent, FireControlKey MachineKey, bool IsActorForLocalPlayer)
{
	//todo, registration goes here.
	ActorKey ParentKey = PointerHash(parent, MachineKey);
	LocalActorToFireControlMapping->Add(ParentKey, MachineKey);

	//this is a hack. this is such a hack. oh god.
	if(IsActorForLocalPlayer)
	{
#if UE_BUILD_SHIPPING != 0
		throw; //I told you not to ship this without checking.
#endif
		//this relies on a really ugly hack using the ENUM. do not ship this without being sure you want to.
		InputStreamKey LocalKey = GetStreamForPlayer(APlayer::CABLE);
		StreamToActorMapping->Add(LocalKey, ParentKey);
		ActorToStreamMapping->Add(ParentKey, LocalKey);
		return TPair<ActorKey, InputStreamKey>(ParentKey, LocalKey);			
	}
	else
	{
		throw;
	}

}


