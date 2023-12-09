#include "DatadogApi.h"
#include "Runtime/JsonUtilities/Public/JsonObjectConverter.h"


static FString SiteToHost(FString& site) {
	if (site.Equals("datadoghq.com")) {
		return "https://app.datadoghq.com";
	}
	else if (site.Equals("us3.datadoghq.com")) {
		return "https://us3.datadoghq.com";
	}
	else if (site.Equals("us5.datadoghq.com")) {
		return "https://us5.datadoghq.com";
	}
	else if (site.Equals("datadoghq.eu")) {
		return "https://app.datadoghq.eu";
	}
	else if (site.Equals("ap1.datadoghq.com")) {
		return "https://ap1.datadoghq.com";
	}
	else {
		return "";
	}
}

UDatadogApi::UDatadogApi()
{
	UE_LOG(LogDatadogMod, Verbose, TEXT("Detected API Key %s"), *ddApiKey);
	GConfig->GetString(TEXT("/Script/DatadogMod.DatadogApi"), TEXT("ddApiKey"), ddApiKey, GGameIni);
	UE_LOG(LogDatadogMod, Verbose, TEXT("Detected API Key %s"), *ddApiKey);
	http = &FHttpModule::Get();
}

void UDatadogApi::Submit(TArray<FDatadogTimeseries> timeseries)
{
	FString jsonString;

	FMetricsPayload payload;
	payload.series = timeseries;
	
	bool success = FJsonObjectConverter::UStructToJsonObjectString(payload, jsonString);
	if (!success) {
		UE_LOG(LogDatadogMod, Error, TEXT("Failed to serialize payload"));
		return;
	}

	// TODO: Check the payload size. Compressed, it must be under 5 MB. Uncompressed, under 512kB.

	TSharedRef<IHttpRequest> Request = http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UDatadogApi::OnResponseReceived);
	
	// Set the Datadog Host
	auto host = SiteToHost(ddSite);
	if (host.Equals("")) {
		UE_LOG(LogDatadogMod, Error, TEXT("Unrecognized datadog site %s"), *ddSite);
		return;
	}
	
	Request->SetURL(host + "/api/v2/series");
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", TEXT("application/json"));
	Request->SetHeader("DD-API-KEY", ddApiKey);
	Request->SetContentAsString(jsonString);
	Request->ProcessRequest();
}

void UDatadogApi::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful) {
		UE_LOG(LogDatadogMod, Error, TEXT("Metrics submission rejected by Datadog: %s"), *Response->GetContentAsString());
	}
}

void UDatadogPayloadBuilder::SetTimestamp(int64 ts)
{
	timestamp = timestamp;
}

void UDatadogPayloadBuilder::SetInterval(int64 newInterval)
{
	interval = newInterval;
}

void UDatadogPayloadBuilder::SetGlobalTags(TArray<FString>& newTags)
{
	tags = newTags;
}

void UDatadogPayloadBuilder::AddGauge(FString name, TArray<FString>& ntags, double value, FString unit)
{
	AddMetric(MetricType::Gauge, name, ntags, value, unit);
}

void UDatadogPayloadBuilder::AddCounter(FString name, TArray<FString>& ntags, double value, FString unit)
{
	AddMetric(MetricType::Count, name, ntags, value, unit);
}

void UDatadogPayloadBuilder::AddRate(FString name, TArray<FString>& ntags, double value, FString unit)
{
	AddMetric(MetricType::Rate, name, ntags, value, unit);
}

TArray<FDatadogTimeseries> UDatadogPayloadBuilder::Build()
{
	TArray<FDatadogTimeseries> builtTimeseries;

	for (auto& it : timeseries) {
		it.interval = interval;
		for (auto& tag : tags) {
			it.tags.Add(tag);
		}

		it.points[0].timestamp = timestamp;
	}

	return timeseries;
}

void UDatadogPayloadBuilder::AddMetric(MetricType type, FString& name, TArray<FString>& ntags, double value, FString unit)
{
	FDatadogTimeseries newTimeseries;
	newTimeseries.metric = name;
	newTimeseries.tags = ntags;
	newTimeseries.unit = unit;
	newTimeseries.type = type;

	FTimeseriesPoint point;
	point.value = value;
	newTimeseries.points.Add(point);

	timeseries.Add(newTimeseries);
}
