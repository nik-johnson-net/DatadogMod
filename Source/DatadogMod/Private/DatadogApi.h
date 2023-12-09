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

UCLASS(Config=Game)
class UDatadogApi : public UObject
{
	GENERATED_BODY()
	
public:
	UDatadogApi();
	void Submit(TArray<FDatadogTimeseries> &timeseries);
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

private:
	FHttpModule* http;

	UPROPERTY(Config)
	FString ddApiKey;
	
	UPROPERTY(Config)
	FString ddSite;
};
