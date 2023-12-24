#include "Datadog.h"
#include "DatadogMod.h"
#include "FGGameState.h"
#include "Collectors/DDCollectorCircuit.h"
#include "Collectors/DDCollectorBuildables.h"
#include "FGCircuitSubsystem.h"
#include "Buildables/FGBuildableCircuitSwitch.h"
#include "FGBuildingTagInterface.h"

UWorld* UDatadog::GetWorld() const
{
	return mWorld;
}

void UDatadog::BeginPlay() {
	GetWorld()->GetTimerManager().SetTimer(statTimerHandle, this, &UDatadog::CollectStats, collectionPeriod, true);
	UE_LOG(LogDatadogMod, Log, TEXT("Datadog Timer started, collecting every %.2f seconds."), collectionPeriod);
}

void UDatadog::Init(UWorld* world)
{
	mWorld = world;
	datadogApi = NewObject<UDatadogApi>();

	// Add Collectors Here
	Collectors.Add(NewObject<UDDCollectorCircuit>());
	Collectors.Add(NewObject<UDDCollectorBuildables>());

	UE_LOG(LogDatadogMod, Log, TEXT("Datadog Collectors Initialized."));
}

void UDatadog::CollectStats() {
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

	// Compute circuit names
	TMap<int32, FString> circuitNames = BuildCircuitNames(world);

	for (auto& collector : Collectors) {
		collector->Collect(world, payloadBuilder, circuitNames);
	}

	datadogApi->Submit(payloadBuilder.Build());

	auto elapsed = (FDateTime::Now() - timeStart).GetTotalSeconds();
	UE_LOG(LogDatadogMod, Verbose, TEXT("Stats collection and submission took %.3f seconds"), elapsed);
}

FString UDatadog::GetCircuitName(UFGCircuit& powerCircuit)
{
	auto sub = powerCircuit.GetOwningCircuitSubsystem();
	for (auto& bridge : sub->mCircuitBridges) {
		// Check if it's a physical power switch
		AFGBuildableCircuitSwitch* circuitSwitch = Cast<AFGBuildableCircuitSwitch>(bridge);
		if (circuitSwitch == nullptr) {
			continue;
		}
		if (circuitSwitch->GetCircuitID1() == powerCircuit.GetCircuitID()) {
			return Cast<IFGBuildingTagInterface>(circuitSwitch)->Execute_GetBuildingTag(circuitSwitch);
		}
	}

	return FString::FString();
}

TMap<int32, FString> UDatadog::BuildCircuitNames(UWorld *world)
{
	TMap<int32, FString> names;

	AFGCircuitSubsystem* circuitSubsystem = AFGCircuitSubsystem::Get(world);
	if (circuitSubsystem == nullptr) {
		UE_LOG(LogDatadogMod, Verbose, TEXT("Could not get circuit subsystem to build circuit names"));
		return names;
	}

	int32 count = 0;
	for (auto& circuit : circuitSubsystem->mCircuits) {
		FString name = GetCircuitName(*circuit.Value);
		if (!name.IsEmpty()) {
			count++;
		}
		names.Add(circuit.Key, name);
	}

	UE_LOG(LogDatadogMod, Verbose, TEXT("Found %d circuits with names."), count);
	
	return names;
}
