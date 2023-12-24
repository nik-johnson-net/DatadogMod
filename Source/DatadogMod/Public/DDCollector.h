#pragma once

#include "CoreMinimal.h"
#include "DatadogPayloadBuilder.h"
#include "DDCollector.generated.h"

UCLASS(Abstract)
class DATADOGMOD_API UDDCollector : public UObject {

	GENERATED_BODY()

public:
	virtual void Collect(UWorld* world, DatadogPayloadBuilder& payloadBuilder, const TMap<int32, FString>& circuitNames) {};
};
