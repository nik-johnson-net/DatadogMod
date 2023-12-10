// Fill out your copyright notice in the Description page of Project Settings.


#include "DatadogModSubsystem.h"
#include "Collectors/DDCollectorCircuit.h"

void ADatadogModSubsystem::BeginPlay() {
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(statTimerHandle, this, &ADatadogModSubsystem::CollectStats, collectionPeriod, true);
}

void ADatadogModSubsystem::Init()
{
	Super::Init();
	datadogApi = NewObject<UDatadogApi>();
	Collectors.Add(NewObject<UDDCollectorCircuit>());
	UE_LOG(LogDatadogMod, Log, TEXT("Datadog Subsystem Initialized."));
}

void ADatadogModSubsystem::CollectStats() {
	auto world = GetWorld();
	if (world == NULL) {
		UE_LOG(LogDatadogMod, Error, TEXT("No world exists."));
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

	for (auto& collector : Collectors) {
		collector->Collect(world, payloadBuilder);
	}

	datadogApi->Submit(payloadBuilder.Build());

	auto elapsed = (FDateTime::Now() - timeStart).GetTotalSeconds();
	UE_LOG(LogDatadogMod, Verbose, TEXT("Stats collection and submission took %.3f seconds"), elapsed);
	
	// Machines:
	//   Check Counts
	//   Check Efficiency

	// Items:
	//   Check Counts
	//   Check Produced
	//   Check Consumed

	// Players:
	//   Check Count

	// Shredder:
	//   Check Current Tickets
	//   Check Point Rate

	// Ticks:
	//   Check Tick Rate
	//   Count Slow / Skipped Ticks
}

/*
void ADatadogModSubsystem::CollectStatistics(UWorld* world, DatadogPayloadBuilder &payloadBuilder)
{
	auto statisticsSubsystem = AFGStatisticsSubsystem::Get(world);
	if (statisticsSubsystem == NULL) {
		UE_LOG(LogDatadogMod, Error, TEXT("Couldn't load StatisticsSubsystem, not reporting, well, a lot of things."));
		return;
	}

	UE_LOG(LogDatadogMod, Verbose, TEXT("mItemsProduced has %d items. mActorsBuiltCount has %d items, mItemsManuallyCraftedCount has %d items."), statisticsSubsystem->mItemsProduced.Num(), statisticsSubsystem->mActorsBuiltCount.Num(), statisticsSubsystem->mItemsManuallyCraftedCount.Num());

	for (auto& production : statisticsSubsystem->mItemsProduced) {
		TArray<FString> tags{ TEXT("item:") + UFGItemDescriptor::GetItemName(production.Key).ToString()};
		auto delta = production.Value - mItemsProduced[production.Key];
		mItemsProduced[production.Key] = production.Value;
		payloadBuilder.AddCounter(TEXT("satisfactory.items.produced"), tags, delta, TEXT("item"));
	}
}
*/