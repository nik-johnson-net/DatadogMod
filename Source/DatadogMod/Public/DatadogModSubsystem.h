// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "DatadogApi.h"
#include "DatadogModSubsystem.generated.h"

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
	void CollectPowerStats(UWorld* world, DatadogPayloadBuilder &payloadBuilder);
	void CollectStatistics(UWorld* world, DatadogPayloadBuilder &payloadBuilder);

	FTimerHandle statTimerHandle;

	UPROPERTY()
	float collectionPeriod = 15.0f;

	UPROPERTY()
	UDatadogApi *datadogApi;
};
