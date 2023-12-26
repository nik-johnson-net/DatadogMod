// Fill out your copyright notice in the Description page of Project Settings.


#include "Collectors/DDCollectorBuildables.h"
#include "DatadogMod.h"
#include "FGBuildableSubsystem.h"
#include "Buildables/FGBuildableFactory.h"
#include "FGStatisticsSubsystem.h"
#include "Buildables/FGBuildableResourceExtractor.h"
#include "Resources/FGResourceDescriptor.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "Buildables/FGBuildableSpaceElevator.h"
#include "Buildables/FGBuildableGenerator.h"
#include "Buildables/FGBuildableResourceSink.h"
#include "FGPowerCircuit.h"
#include "FGPowerInfoComponent.h"

static float calculateResourcePerMinute(const TSubclassOf<UFGRecipe> recipe, const AFGBuildableManufacturer *manufacturer, const FItemAmount& amount)
{
	EResourceForm form = UFGItemDescriptor::GetForm(amount.ItemClass);
	float cyclesPerMinute = 60.0f / manufacturer->GetProductionCycleTime();
	float resourcesPerMinute = amount.Amount * manufacturer->GetProductivity() * cyclesPerMinute;

	// Units are Liters, but in game is m^3 so divide by 1000.
	if (form == EResourceForm::RF_LIQUID || form == EResourceForm::RF_GAS) {
		resourcesPerMinute = resourcesPerMinute / 1000;
	}

	return resourcesPerMinute;
}

void UDDCollectorBuildables::Collect(UWorld* world, DatadogPayloadBuilder& payloadBuilder, const TMap<int32, FString>& circuitNames)
{
	auto buildableSubsystem = AFGBuildableSubsystem::Get(world);
	if (buildableSubsystem == nullptr) {
		UE_LOG(LogDatadogMod, Error, TEXT("Can't load BuildableSubsystem, not reporting building stats."));
		return;
	}

	TArray<AFGBuildableFactory*> factories;
	buildableSubsystem->GetTypedBuildable(factories);

	for (auto& factory : factories) {
		TArray<FString> tags{ "building:" + factory->mDisplayName.ToString() };
		if (factory->HasPower()) {
			tags.Add("powered:true");
		}
		else {
			tags.Add("powered:false");
		}

		// Add circuit name to tags
		UFGPowerInfoComponent* powerInfo = factory->GetPowerInfo();
		if (powerInfo != nullptr) {
			if (powerInfo->GetPowerCircuit() != nullptr) {
				const FString* name = circuitNames.Find(powerInfo->GetPowerCircuit()->GetCircuitID());
				if (name != nullptr) {
					tags.Add(TEXT("circuit:") + *name);
				}
			}

			float consumption = powerInfo->GetActualConsumption();
			payloadBuilder.AddHistogram("satisfactory.building.power", tags, consumption, "megawatt");
		}

		payloadBuilder.AddHistogram(TEXT("satisfactory.building.productivity"), tags, factory->GetProductivity(), "fraction");
		
		if (factory->HasPower() && factory->IsConfigured()) {
			// Check if this factory is a Resource Extractor (ex. Miner, Water Pump)
			AFGBuildableResourceExtractor* resourceExtractor = Cast<AFGBuildableResourceExtractor>(factory);
			if (resourceExtractor != NULL) {
				auto extractedPerMinute = resourceExtractor->GetExtractionPerMinute() * factory->GetProductivity();
				auto name = UFGResourceDescriptor::GetItemName(resourceExtractor->GetExtractableResource()->GetResourceClass());

				tags.Add("item:" + name.ToString());
				payloadBuilder.AddHistogram(TEXT("satisfactory.building.production"), tags, extractedPerMinute, "items per minute");
				
				continue;
			}

			// Check if this factory is a Manufacturer (ex. Constructor, Assembler, Smelter)
			AFGBuildableManufacturer* manufacturer = Cast<AFGBuildableManufacturer>(factory);
			if (manufacturer != NULL) {
				auto recipe = manufacturer->GetCurrentRecipe();
				auto outputs = UFGRecipe::GetProducts(recipe);
				for (auto& output : outputs) {
					auto name = UFGItemDescriptor::GetItemName(output.ItemClass);
					
					float resourcePerMinute = calculateResourcePerMinute(recipe, manufacturer, output);
					
					auto copyTags = TArray<FString>(tags);
					copyTags.Add("item:" + name.ToString());

					payloadBuilder.AddHistogram(TEXT("satisfactory.building.production"), copyTags, resourcePerMinute, "items per minute");
				}

				auto inputs = UFGRecipe::GetIngredients(recipe);
				for (auto& input : inputs) {
					auto name = UFGItemDescriptor::GetItemName(input.ItemClass);

					float resourcePerMinute = calculateResourcePerMinute(recipe, manufacturer, input);

					auto copyTags = TArray<FString>(tags);
					copyTags.Add("item:" + name.ToString());

					payloadBuilder.AddHistogram(TEXT("satisfactory.building.consumption"), copyTags, resourcePerMinute, "items per minute");
				}

				continue;
			}

			// If AFGBuildableSpaceElevator
			AFGBuildableSpaceElevator* spaceElevator = Cast<AFGBuildableSpaceElevator>(factory);
			if (spaceElevator != NULL) {
				continue;
			}

			// If AFGBuildableResourceSink
			AFGBuildableResourceSink* resourceSink = Cast<AFGBuildableResourceSink>(factory);
			if (resourceSink != NULL) {
				continue;
			}

			// If AFGBuildableGenerator
			AFGBuildableGenerator* generator = Cast<AFGBuildableGenerator>(factory);
			if (generator != NULL) {
				continue;
			}

			// If AFGBuildableTrainPlatform
		}
	}
}
