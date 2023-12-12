// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DatadogModSubsystem.h"
#include "DDCollectorBuildables.generated.h"

/**
 * 
 */
UCLASS()
class UDDCollectorBuildables : public UDDCollectionPlugin
{
	GENERATED_BODY()
	
public:
	virtual void Collect(UWorld* world, DatadogPayloadBuilder& payloadBuilder) override;
};
