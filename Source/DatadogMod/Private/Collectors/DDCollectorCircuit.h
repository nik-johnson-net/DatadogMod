#pragma once

#include "DatadogModSubsystem.h"
#include "DDCollectorCircuit.generated.h"

UCLASS()
class UDDCollectorCircuit : public UDDCollectionPlugin
{
	GENERATED_BODY()

public:
	virtual void Collect(UWorld* world, DatadogPayloadBuilder& payloadBuilder) override;
};
