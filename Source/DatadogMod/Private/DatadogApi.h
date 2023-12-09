#pragma once

#include "CoreMinimal.h"
#include "DatadogMod.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "DatadogApi.generated.h"

UENUM()
enum MetricType {
	Unspecified,
	Count,
	Rate,
	Gauge,
};

USTRUCT()
struct FTimeseriesPoint {
	
	GENERATED_BODY()

	int64 timestamp;
	double value;
};

USTRUCT()
struct FDatadogTimeseries {
	
	GENERATED_BODY()
	
	int64 interval;
	FString metric;
	TArray<FString> tags;
	FString unit;
	TArray<FTimeseriesPoint> points;
	MetricType type;
};


USTRUCT()
struct FMetricsPayload {
	GENERATED_BODY()

	TArray<FDatadogTimeseries> series;
};

UCLASS()
class UDatadogPayloadBuilder : public UObject {
	GENERATED_BODY()

public:
	void SetTimestamp(int64 timestamp);
	void SetInterval(int64 interval);
	void SetGlobalTags(TArray<FString> &tags);
	void AddGauge(FString name, TArray<FString>& tags, double value, FString unit = "");
	void AddCounter(FString name, TArray<FString>& tags, double value, FString unit = "");
	void AddRate(FString name, TArray<FString>& tags, double value, FString unit = "");
	TArray<FDatadogTimeseries> Build();

private:
	void AddMetric(MetricType type, FString& name, TArray<FString>& tags, double value, FString unit = "");
	int64 timestamp;
	int64 interval;
	TArray<FDatadogTimeseries> timeseries;
	TArray<FString> tags;
};

UCLASS(config=Game)
class UDatadogApi : public UObject
{
	GENERATED_BODY()
	
public:
	UDatadogApi();
	void Submit(TArray<FDatadogTimeseries> timeseries);
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

private:
	FHttpModule* http;

	UPROPERTY(Config)
	FString ddApiKey;
	
	UPROPERTY(Config)
	FString ddSite;
};
