#pragma once

#include "DatadogModSubsystem.h"
#include "UDDCollectorCircuit.generated.h"

UCLASS()
class UDDCollectorCircuit : public UDDCollectionPlugin
{
	GENERATED_BODY()

public:
	void Collect(UWorld* world, DatadogPayloadBuilder& payloadBuilder);
};
