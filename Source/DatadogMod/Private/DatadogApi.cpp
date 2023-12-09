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
	UE_LOG(LogDatadogMod, Error, TEXT("Detected API Key %s"), *ddApiKey);
	http = &FHttpModule::Get();
}

void UDatadogApi::Submit(TArray<FDatadogTimeseries> &timeseries)
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

