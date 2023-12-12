// Fill out your copyright notice in the Description page of Project Settings.


#include "Collectors/DDCollectorBuildables.h"
#include "DatadogMod.h"
#include "FGBuildableSubsystem.h"
#include "Buildables/FGBuildableFactory.h"
#include "FGStatisticsSubsystem.h"
#include "Buildables/FGBuildableResourceExtractor.h"
#include "Resources/FGResourceDescriptor.h"
#include "Buildables/FGBuildableManufacturer.h"

struct Stat {
	 
};

void UDDCollectorBuildables::Collect(UWorld* world, DatadogPayloadBuilder& payloadBuilder)
{
	auto buildableSubsystem = AFGBuildableSubsystem::Get(world);

	TArray<AFGBuildableFactory*> factories;
	buildableSubsystem->GetTypedBuildable(factories);

	for (auto& factory : factories) {
		TArray<FString> tags{ "id:" + FString::FromInt(factory->GetUniqueID()), "building:" + factory->mDisplayName.ToString()};
		payloadBuilder.AddGauge(TEXT("satisfactory.building.productivity"), tags, factory->GetProductivity(), "percent");
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
				payloadBuilder.AddGauge(TEXT("satisfactory.building.production"), tags, extractedPerMinute, "item");
				
				continue;
			}

			// Check if this factory is a Manufacturer (ex. Constructor, Assembler, Smelter)
			AFGBuildableManufacturer* manufacturer = Cast<AFGBuildableManufacturer>(factory);
			if (manufacturer != NULL) {
				auto recipe = manufacturer->GetCurrentRecipe();
				auto outputs = UFGRecipe::GetProducts(recipe);
				for (auto& output : outputs) {
					auto name = UFGResourceDescriptor::GetItemName(output.ItemClass);
					// Avg Recipe Output Per Minute = Amount * ((Duration * Machine Speed) / 60 seconds) * Machine Efficiency
					auto resourcePerMinute = output.Amount * manufacturer->GetProductivity() * (UFGRecipe::GetManufacturingDuration(recipe) * manufacturer->GetManufacturingSpeed() / 60.0f);
					
					auto copyTags = TArray<FString>(tags);
					copyTags.Add("item:" + name.ToString());
					payloadBuilder.AddGauge(TEXT("satisfactory.building.production"), tags, resourcePerMinute, "item");
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
