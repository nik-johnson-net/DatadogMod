// Fill out your copyright notice in the Description page of Project Settings.


#include "Collectors/DDCollectorBuildables.h"
#include "DatadogMod.h"
#include "FGBuildableSubsystem.h"
#include "Buildables/FGBuildableFactory.h"
#include "FGStatisticsSubsystem.h"
#include "Buildables/FGBuildableResourceExtractor.h"
#include "Resources/FGResourceDescriptor.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "FGPowerCircuit.h"
#include "FGPowerInfoComponent.h"

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
		TArray<FString> tags{ "id:" + FString::FromInt(factory->GetUniqueID()), "building:" + factory->mDisplayName.ToString() };

		UFGPowerInfoComponent* powerInfo = factory->GetPowerInfo();
		if (powerInfo != nullptr) {
			const FString *name = circuitNames.Find(powerInfo->GetPowerCircuit()->GetCircuitID());
			if (name != nullptr) {
				tags.Add(TEXT("circuit:") + *name);
			}
		}

		payloadBuilder.AddGauge(TEXT("satisfactory.building.productivity"), tags, factory->GetProductivity(), "fraction");
		payloadBuilder.AddGauge(TEXT("satisfactory.building.powered"), tags, factory->HasPower(), "boolean");
		
		if (factory->HasPower() && factory->IsConfigured()) {
			factory->GetProductionCycleTime();
			factory->GetProductivity();

			// Check if this factory is a Resource Extractor (ex. Miner, Water Pump)
			AFGBuildableResourceExtractor* resourceExtractor = Cast<AFGBuildableResourceExtractor>(factory);
			if (resourceExtractor != NULL) {
				auto extractedPerMinute = resourceExtractor->GetExtractionPerMinute() * factory->GetProductivity();
				auto name = UFGResourceDescriptor::GetItemName(resourceExtractor->GetExtractableResource()->GetResourceClass());
				
				tags.Add("item:" + name.ToString());
				payloadBuilder.AddGauge(TEXT("satisfactory.building.production"), tags, extractedPerMinute, "items per minute");
				
				continue;
			}

			// Check if this factory is a Manufacturer (ex. Constructor, Assembler, Smelter)
			AFGBuildableManufacturer* manufacturer = Cast<AFGBuildableManufacturer>(factory);
			if (manufacturer != NULL) {
				auto recipe = manufacturer->GetCurrentRecipe();
				auto outputs = UFGRecipe::GetProducts(recipe);
				for (auto& output : outputs) {
					auto name = UFGItemDescriptor::GetItemName(output.ItemClass);
					auto form = UFGItemDescriptor::GetForm(output.ItemClass);
					
					// Avg Recipe Output Per Minute = Amount * ((Duration * Machine Speed) / 60 seconds) * Machine Efficiency
					auto resourcePerMinute = output.Amount * manufacturer->GetProductivity() * (UFGRecipe::GetManufacturingDuration(recipe) * manufacturer->GetManufacturingSpeed() / 60.0f);
					
					auto copyTags = TArray<FString>(tags);
					copyTags.Add("item:" + name.ToString());

					payloadBuilder.AddGauge(TEXT("satisfactory.building.production"), tags, resourcePerMinute, "items per minute");
				}

				auto inputs = UFGRecipe::GetIngredients(recipe);
				for (auto& input : inputs) {
					auto name = UFGItemDescriptor::GetItemName(input.ItemClass);
					auto form = UFGItemDescriptor::GetForm(input.ItemClass);

					// Avg Recipe Input Per Minute = Amount * ((Duration * Machine Speed) / 60 seconds) * Machine Efficiency
					auto resourcePerMinute = input.Amount * manufacturer->GetProductivity() * (UFGRecipe::GetManufacturingDuration(recipe) * manufacturer->GetManufacturingSpeed() / 60.0f);

					auto copyTags = TArray<FString>(tags);
					copyTags.Add("item:" + name.ToString());

					payloadBuilder.AddGauge(TEXT("satisfactory.building.consumption"), tags, resourcePerMinute, "items per minute");
				}

				continue;
			}

			// If AFGBuildableTradingPost

			// If AFGBuildableTrainPlatform

			// If AFGBuildableResourceSink

			// If AFGBuildableGenerator
		}
	}
}
