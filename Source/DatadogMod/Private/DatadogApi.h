#pragma once

#include "CoreMinimal.h"
#include "DatadogMod.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "DatadogApi.generated.h"

UENUM()
enum class EMetricType : uint8 {
	Unspecified = 0,
	Count = 1,
	Rate = 2,
	Gauge = 3,
};

USTRUCT()
struct FTimeseriesPoint {
	
	GENERATED_BODY()

	UPROPERTY()
	int64 timestamp;

	UPROPERTY()
	double value;
};

USTRUCT()
struct FDatadogTimeseries {
	
	GENERATED_BODY()
	
	UPROPERTY()
	int64 interval;

	UPROPERTY()
	FString metric;

	UPROPERTY()
	TArray<FString> tags;

	UPROPERTY()
	FString unit;

	UPROPERTY()
	TArray<FTimeseriesPoint> points;

	UPROPERTY()
	EMetricType type;
};


USTRUCT()
struct FMetricsPayload {
	GENERATED_BODY()

	UPROPERTY()
	TArray<FDatadogTimeseries> series;
};

class DatadogPayloadBuilder {
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

UCLASS(config=Game)
class UDatadogApi : public UObject
{
	GENERATED_BODY()
	
public:
	UDatadogApi();
	void Submit(TArray<FDatadogTimeseries> timeseries);
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

private:
	UPROPERTY(Config)
	FString DatadogApiKey;
	
	UPROPERTY(Config)
	FString DatadogSite;
	
	FString mHost;
};
