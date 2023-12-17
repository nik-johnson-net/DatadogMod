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
struct DATADOGMOD_API FTimeseriesPoint {
	
	GENERATED_BODY()

	UPROPERTY()
	int64 timestamp;

	UPROPERTY()
	double value;
};

USTRUCT()
struct DATADOGMOD_API FDatadogTimeseries {
	
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
struct DATADOGMOD_API FMetricsPayload {
	GENERATED_BODY()

	UPROPERTY()
	TArray<FDatadogTimeseries> series;
};

UCLASS(config=Game)
class DATADOGMOD_API UDatadogApi : public UObject
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
