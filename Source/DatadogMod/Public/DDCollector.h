#pragma once

#include "CoreMinimal.h"
#include "DatadogApi.h"
#include "DDCollector.generated.h"

class DATADOGMOD_API DatadogPayloadBuilder {
public:
	void SetTimestamp(int64 timestamp);
	void SetInterval(int64 interval);
	void SetGlobalTags(TArray<FString> &tags);
	void AddGauge(FString name, TArray<FString>& tags, double value, FString unit = "");
	void AddCounter(FString name, TArray<FString>& tags, double value, FString unit = "");
	void AddRate(FString name, TArray<FString>& tags, double value, FString unit = "");
	TArray<FDatadogTimeseries> Build();

private:
	void AddMetric(EMetricType type, FString& name, TArray<FString>& tags, double value, FString unit = "");
	int64 timestamp;
	int64 interval;
	TArray<FDatadogTimeseries> timeseries;
	TArray<FString> tags;
};

UCLASS(Abstract)
class DATADOGMOD_API UDDCollector : public UObject {

	GENERATED_BODY()

public:
	virtual void Collect(UWorld* world, DatadogPayloadBuilder& payloadBuilder, const TMap<int32, FString>& circuitNames) {};
};
