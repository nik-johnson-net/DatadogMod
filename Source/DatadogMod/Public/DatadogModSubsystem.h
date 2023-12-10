// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "DatadogApi.h"
#include "DatadogModSubsystem.generated.h"

UCLASS()
class DATADOGMOD_API UDDCollectionPlugin : public UObject {

	GENERATED_BODY()

public:
	virtual void Collect(UWorld* world, DatadogPayloadBuilder& payloadBuilder) {};
};

/**
 * 
 */
UCLASS()
class DATADOGMOD_API ADatadogModSubsystem : public AModSubsystem
{
	GENERATED_BODY()
	
private:
	virtual void BeginPlay() override;
	void CollectStats();

	FTimerHandle statTimerHandle;

	UPROPERTY()
	float collectionPeriod = 15.0f;

	UPROPERTY()
	UDatadogApi *datadogApi;

	UPROPERTY()
	TArray<UDDCollectionPlugin*> Collectors;
};
