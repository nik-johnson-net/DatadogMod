#include "Collectors/DDCollectorCircuit.h"
#include "Buildables/FGBuildableCircuitSwitch.h"
#include "Buildables/FGBuildablePriorityPowerSwitch.h"
#include "FGCircuitSubsystem.h"
#include "FGPowerCircuit.h"

void UDDCollectorCircuit::Collect(UWorld* world, DatadogPayloadBuilder& payloadBuilder, const TMap<int32, FString>& circuitNames)
{
	auto circuitSubsystem = AFGCircuitSubsystem::Get(world);
	if (circuitSubsystem == NULL) {
		UE_LOG(LogDatadogMod, Error, TEXT("Can't load CircuitSubsystem, not reporting power stats."));
		return;
	}

	// Pull stats for Power Switches.
	for (auto& it : circuitSubsystem->mCircuitBridges) {
		// Check if it's a physical power switch
		AFGBuildableCircuitSwitch* circuitSwitch = Cast<AFGBuildableCircuitSwitch>(it);
		if (circuitSwitch != NULL) {
			FString name = Cast<IFGBuildingTagInterface>(circuitSwitch)->Execute_GetBuildingTag(circuitSwitch);
			TArray<FString> tags{ TEXT("switch_name:") + name };

			AFGBuildablePriorityPowerSwitch* priorityPowerSwitch = Cast<AFGBuildablePriorityPowerSwitch>(circuitSwitch);
			if (priorityPowerSwitch != NULL) {
				// Record Priority Switch Priority
				payloadBuilder.AddGauge(TEXT("satisfactory.power.switch.priority"), tags, priorityPowerSwitch->GetPriority());
			}

			// Record Switch State
			payloadBuilder.AddGauge(TEXT("satisfactory.power.switch.on"), tags, circuitSwitch->IsSwitchOn(), "boolean");
		}
	}

	// Pull stats for all circuits.
	 for (auto& it : circuitSubsystem->mCircuits) {
		// Cast to a Power Circuit. If it's not, skip. Right now there are only power circuits, so this should always pass.
		UFGPowerCircuit * powerCircuit = Cast<UFGPowerCircuit>(it.Value);
		if (powerCircuit == NULL) {
			UE_LOG(LogDatadogMod, Warning, TEXT("Failed to cast circuit id %d to UFGPowerCircuit"), it.Value->GetCircuitID());
			continue;
		}
		
		FPowerCircuitStats circuitStats;
		powerCircuit->GetStats(circuitStats);
		
		TArray<FString> tags{ "circuit_id:" + FString::FromInt(powerCircuit->GetCircuitID()) };
		const FString* name = circuitNames.Find(powerCircuit->GetCircuitID());
		if (name != nullptr) {
			tags.Add(TEXT("circuit:") + *name);
		}
		payloadBuilder.AddGauge(TEXT("satisfactory.power.produced"), tags, circuitStats.PowerProduced, "megawatt");
		payloadBuilder.AddGauge(TEXT("satisfactory.power.production_capacity"), tags, circuitStats.PowerProductionCapacity, "megawatt");
		payloadBuilder.AddGauge(TEXT("satisfactory.power.consumed"), tags, circuitStats.PowerConsumed, "megawatt");
		payloadBuilder.AddGauge(TEXT("satisfactory.power.consumption_maximum"), tags, circuitStats.MaximumPowerConsumption, "megawatt");
		payloadBuilder.AddGauge(TEXT("satisfactory.power.battery_input"), tags, circuitStats.BatteryPowerInput, "megawatt");
		payloadBuilder.AddGauge(TEXT("satisfactory.power.battery_stored"), tags, powerCircuit->GetBatterySumPowerStore(), "megawatt-hour");
		payloadBuilder.AddGauge(TEXT("satisfactory.power.battery_capacity"), tags, powerCircuit->GetBatterySumPowerStoreCapacity(), "megawatt-hour");
	}

	// Stats for circuit groups.
	// Circuit Groups are a server-side grouping of all circuits that are simulated as one unit.
	// IE. Two power grids connected by a switch. When the switch is on, there is one circuit group.
	// When the switch is off, there are two circuit groups.
	// Circuit Groups do not have a unique identifier, so it's hard to submit metrics for each
	// and have a coherent picture before/after switch changes.
	//
	// Think about a layout of the following:
	// [ power plant ] -> [ switch ] -> [ distribution grid ] -> [ switches A, B, C ] -> [ plant A, B, C ]
	// If the switch to B is tripped, what can we use to identify the new group that will be created?
	//
	// One option is to use the group uobject ID, but it is not persistent across game instances.
	for (auto& it : circuitSubsystem->mCircuitGroups) {
		// Check if it's a power circuit group
		UFGPowerCircuitGroup* powerCircuitGroup = Cast<UFGPowerCircuitGroup>(it);
		if (powerCircuitGroup == nullptr) {
			continue;
		}

		auto batteryPowerInput = powerCircuitGroup->mCircuits[0]->GetBatterySumPowerInput();

		TArray<FString> tags{ "circuit_group_id:" + FString::FromInt(powerCircuitGroup->GetUniqueID()) };
		payloadBuilder.AddGauge(TEXT("satisfactory.power.group.produced"), tags, powerCircuitGroup->mBaseProduction, "megawatt");
		payloadBuilder.AddGauge(TEXT("satisfactory.power.group.production_capacity"), tags, powerCircuitGroup->mMaximumProductionCapacity, "megawatt");
		payloadBuilder.AddGauge(TEXT("satisfactory.power.group.consumed"), tags, powerCircuitGroup->mConsumption, "megawatt");
		payloadBuilder.AddGauge(TEXT("satisfactory.power.group.consumption_maximum"), tags, powerCircuitGroup->mMaximumPowerConsumption, "megawatt");
		payloadBuilder.AddGauge(TEXT("satisfactory.power.group.battery_input"), tags, batteryPowerInput, "megawatt");
		payloadBuilder.AddGauge(TEXT("satisfactory.power.group.battery_stored"), tags, powerCircuitGroup->mTotalPowerStore, "megawatt-hour");
		payloadBuilder.AddGauge(TEXT("satisfactory.power.group.battery_capacity"), tags, powerCircuitGroup->mTotalPowerStoreCapacity, "megawatt-hour");
	}
}
