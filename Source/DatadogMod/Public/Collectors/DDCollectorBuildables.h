// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DDCollector.h"
#include "DDCollectorBuildables.generated.h"

/**
 * 
 */
UCLASS()
class DATADOGMOD_API UDDCollectorBuildables : public UDDCollector
{
	GENERATED_BODY()
	
public:
	virtual void Collect(UWorld* world, DatadogPayloadBuilder& payloadBuilder) override;
};
