// Fill out your copyright notice in the Description page of Project Settings.


#include "DatadogModSubsystem.h"
#include "FGGameState.h"
#include "Collectors/DDCollectorCircuit.h"
#include "Collectors/DDCollectorBuildables.h"

void ADatadogModSubsystem::BeginPlay() {
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(statTimerHandle, this, &ADatadogModSubsystem::CollectStats, collectionPeriod, true);
}

void ADatadogModSubsystem::Init()
{
	Super::Init();
	datadogApi = NewObject<UDatadogApi>();
	
	// Add Collectors Here
	Collectors.Add(NewObject<UDDCollectorCircuit>());
	Collectors.Add(NewObject< UDDCollectorBuildables>());
	
	UE_LOG(LogDatadogMod, Log, TEXT("Datadog Subsystem Initialized."));
}

void ADatadogModSubsystem::CollectStats() {
	auto world = GetWorld();
	if (world == NULL) {
		UE_LOG(LogDatadogMod, Error, TEXT("No world exists."));
		return;
	}

	auto gameState = Cast<AFGGameState>(world->GetGameState());
	if (gameState == NULL) {
		UE_LOG(LogDatadogMod, Error, TEXT("Could not get game state."));
		return;
	}

	// Don't do anything if the game is paused.
	if (world->IsPaused()) {
		UE_LOG(LogDatadogMod, Verbose, TEXT("Not collecting stats because the world is paused."));
		return;
	}

	auto timeStart = FDateTime::Now();
	UE_LOG(LogDatadogMod, Verbose, TEXT("Collecting stats."));
	
	DatadogPayloadBuilder payloadBuilder;
	payloadBuilder.SetInterval(collectionPeriod);
	payloadBuilder.SetTimestamp(FDateTime::UtcNow().ToUnixTimestamp());
	TArray<FString> tags{ "session_name:" + gameState->GetSessionName() };
	payloadBuilder.SetGlobalTags(tags);

	for (auto& collector : Collectors) {
		collector->Collect(world, payloadBuilder);
	}

	datadogApi->Submit(payloadBuilder.Build());

	auto elapsed = (FDateTime::Now() - timeStart).GetTotalSeconds();
	UE_LOG(LogDatadogMod, Verbose, TEXT("Stats collection and submission took %.3f seconds"), elapsed);
}
