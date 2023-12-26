#pragma once

#include "CoreMinimal.h"
#include "DatadogApi.h"

struct DATADOGMOD_API DatadogHistogram {
	FString name;
	TArray<FString> tags;
	TArray<double> values;
	FString unit;

	FString ToString();
};

class DATADOGMOD_API DatadogPayloadBuilder {
public:
	void SetTimestamp(int64 timestamp);
	void SetInterval(int64 interval);
	void SetGlobalTags(TArray<FString>& tags);
	void AddGauge(FString name, TArray<FString>& tags, double value, FString unit = "");
	void AddCounter(FString name, TArray<FString>& tags, double value, FString unit = "");
	void AddRate(FString name, TArray<FString>& tags, double value, FString unit = "");
	void AddHistogram(FString name, TArray<FString>& tags, double value, FString unit = "");
	TArray<FDatadogTimeseries> Build();

private:
	void AddMetric(EMetricType type, const FString& name, TArray<FString>& tags, double value, FString unit = "");
	void BuildHistograms();
	int64 timestamp;
	int64 interval;
	TArray<FDatadogTimeseries> timeseries;
	TArray<FString> tags;

	TMap<FString, DatadogHistogram> histograms;
};
