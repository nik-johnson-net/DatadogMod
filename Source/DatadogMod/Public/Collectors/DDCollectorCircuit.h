#pragma once

#include "DDCollector.h"
#include "DDCollectorCircuit.generated.h"

UCLASS()
class DATADOGMOD_API UDDCollectorCircuit : public UDDCollector
{
	GENERATED_BODY()

public:
	virtual void Collect(UWorld* world, DatadogPayloadBuilder& payloadBuilder, const TMap<int32, FString>& circuitNames) override;
};
