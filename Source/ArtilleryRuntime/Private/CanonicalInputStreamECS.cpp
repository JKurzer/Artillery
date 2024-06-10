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
		SingletonPatternMatcher = MakeShareable<UCanonicalInputStreamECS::FConservedInputPatternMatcher>( new UCanonicalInputStreamECS::FConservedInputPatternMatcher());
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


bool UCanonicalInputStreamECS::registerPattern(TSharedPtr<FActionPattern> ToBind, FActionBitMask ToSeek, FGunKey ToFire, ActorKey FCM_Owner_Actor)
{

	//this should modify the singleton PatternMatcher
	return false;
}

bool UCanonicalInputStreamECS::removePattern(TSharedPtr<FActionPattern> ToBind, FActionBitMask ToSeek, FGunKey ToFire, ActorKey FCM_Owner_Actor)
{
	//this should modify the singleton PatternMatcher
	return false;
}

ActorKey UCanonicalInputStreamECS::registerFCMKeyToParentActorMapping(AActor* parent, FireControlKey MyKey)
{
	//todo, registration goes here.
	return 0;
}
