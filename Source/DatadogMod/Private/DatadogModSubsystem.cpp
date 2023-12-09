// Fill out your copyright notice in the Description page of Project Settings.


#include "DatadogModSubsystem.h"
#include "FactoryStatHelpers.h"
#include "FGCircuitSubsystem.h"
#include "DatadogMod.h"
#include "FGPowerCircuit.h"
#include "Buildables/FGBuildableCircuitSwitch.h"
#include "Buildables/FGBuildablePriorityPowerSwitch.h"
#include "FGStatisticsSubsystem.h"

void ADatadogModSubsystem::BeginPlay() {
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(statTimerHandle, this, &ADatadogModSubsystem::CollectStats, collectionPeriod, true);
	UE_LOG(LogDatadogMod, Verbose, TEXT("Datadog Subsystem Initialized."));
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

	UE_LOG(LogDatadogMod, Verbose, TEXT("Collecting stats."));
	CollectPowerStats(world);
	CollectStatistics(world);
	
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

void ADatadogModSubsystem::CollectPowerStats(UWorld* world)
{
	auto circuitSubsystem = AFGCircuitSubsystem::Get(world);
	if (circuitSubsystem == NULL) {
		UE_LOG(LogDatadogMod, Error, TEXT("Can't load CircuitSubsystem, not reporting power stats."));
		return;
	}
	
	// Pull stats for all circuits.
	for (auto& it : circuitSubsystem->mCircuits) {
		// Cast to a Power Circuit. If it's not, skip. Right now there are only power circuits, so this should always pass.
		UFGPowerCircuit* powerCircuit = Cast<UFGPowerCircuit>(it.Value);
		if (powerCircuit == NULL) {
			UE_LOG(LogDatadogMod, Verbose, TEXT("Failed to cast circuit id %d to UFGPowerCircuit"), it.Value->GetCircuitID());
			continue;
		}

		FPowerCircuitStats circuitStats;
		powerCircuit->GetStats(circuitStats);

		// Log some useful info.
		// TODO: Replace with better accessors on powerCircuit
		UE_LOG(LogDatadogMod, Verbose, TEXT("Power Stats: id %d, produced %.2fMW, consumed %.2fMW, capacity %.2fMW, max consumption %.2fMW, battery charging %.2fMW"), powerCircuit->GetCircuitID(), circuitStats.PowerProduced, circuitStats.PowerConsumed, circuitStats.PowerProductionCapacity, circuitStats.MaximumPowerConsumption, circuitStats.BatteryPowerInput);
	}
	
	// Pull stats for Power Switches.
	for (auto& it : circuitSubsystem->mCircuitBridges) {
		// Check if it's a physical power switch
		AFGBuildableCircuitSwitch* circuitSwitch = Cast<AFGBuildableCircuitSwitch>(it);
		if (circuitSwitch != NULL) {
			AFGBuildablePriorityPowerSwitch* priorityPowerSwitch = Cast<AFGBuildablePriorityPowerSwitch>(circuitSwitch);
			if (priorityPowerSwitch != NULL) {
				// Is there more we can do with a Priority Switch?
			}

			// Record Switch State
			UE_LOG(LogDatadogMod, Verbose, TEXT("Power Switch %s is %b"), circuitSwitch->GetBuildingTag(), circuitSwitch->IsSwitchOn());
		}

	}
}

void ADatadogModSubsystem::CollectStatistics(UWorld* world)
{
	auto statisticsSubsystem = AFGStatisticsSubsystem::Get(world);
	if (statisticsSubsystem == NULL) {
		UE_LOG(LogDatadogMod, Error, TEXT("Couldn't load StatisticsSubsystem, not reporting, well, a lot of things."));
		return;
	}

	UE_LOG(LogDatadogMod, Verbose, TEXT("Got stats"));
}
