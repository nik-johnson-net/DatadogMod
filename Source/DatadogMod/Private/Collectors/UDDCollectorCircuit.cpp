#include "UDDCollectorCircuit.h"
#include "Buildables/FGBuildableCircuitSwitch.h"
#include "Buildables/FGBuildablePriorityPowerSwitch.h"
#include "FGCircuitSubsystem.h"
#include "FGPowerCircuit.h"

void UDDCollectorCircuit::Collect(UWorld* world, DatadogPayloadBuilder& payloadBuilder)
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

		TArray<FString> tags{ "circuit_id:" + FString::FromInt(powerCircuit->GetCircuitID()) };
		payloadBuilder.AddGauge(FString(TEXT("satisfactory.power.produced")), tags, circuitStats.PowerProduced, "MW");
		payloadBuilder.AddGauge(FString(TEXT("satisfactory.power.production_capacity")), tags, circuitStats.PowerProductionCapacity, "MW");
		payloadBuilder.AddGauge(FString(TEXT("satisfactory.power.consumed")), tags, circuitStats.PowerConsumed, "MW");
		payloadBuilder.AddGauge(FString(TEXT("satisfactory.power.consumption_maximum")), tags, circuitStats.MaximumPowerConsumption, "MW");
		payloadBuilder.AddGauge(FString(TEXT("satisfactory.power.battery_input")), tags, circuitStats.BatteryPowerInput, "MW");
		payloadBuilder.AddGauge(FString(TEXT("satisfactory.power.battery_stored")), tags, powerCircuit->GetBatterySumPowerStore(), "MW");
		payloadBuilder.AddGauge(FString(TEXT("satisfactory.power.battery_capacity")), tags, powerCircuit->GetBatterySumPowerStoreCapacity(), "MW");
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
			UE_LOG(LogDatadogMod, Verbose, TEXT("Power Switch %s is %b"), *circuitSwitch->GetBuildingTag(), circuitSwitch->IsSwitchOn());
			TArray<FString> tags{ TEXT("switch_name:") + circuitSwitch->GetBuildingTag() };
			payloadBuilder.AddGauge(TEXT("satisfactory.power.switch"), tags, circuitSwitch->IsSwitchOn());
		}
	}
}
