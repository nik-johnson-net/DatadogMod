#include "DatadogApi.h"
#include "zlib/1.2.12/include/zlib.h"
#include "Runtime/JsonUtilities/Public/JsonObjectConverter.h"

static const FString SiteToHost(FString& site) {
	if (site.Equals("datadoghq.com")) {
		return TEXT("https://api.datadoghq.com");
	}
	else if (site.Equals("us3.datadoghq.com")) {
		return TEXT("https://api.us3.datadoghq.com");
	}
	else if (site.Equals("us5.datadoghq.com")) {
		return TEXT("https://api.us5.datadoghq.com");
	}
	else if (site.Equals("datadoghq.eu")) {
		return TEXT("https://api.datadoghq.eu");
	}
	else if (site.Equals("ap1.datadoghq.com")) {
		return TEXT("https://api.ap1.datadoghq.com");
	}
	else {
		return TEXT("");
	}
}


const FJsonObjectConverter::CustomExportCallback ExportEnumAsNumber =
FJsonObjectConverter::CustomExportCallback::CreateLambda(
	[](FProperty* Property, const void* Value) -> TSharedPtr<FJsonValue>
	{
		if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
		{
			// export enums as strings
			UEnum* EnumDef = EnumProperty->GetEnum();
			auto NumberValue = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(Value);
			return MakeShared<FJsonValueNumber>(NumberValue);
		}
		return {};
	});

UDatadogApi::UDatadogApi()
{
	GConfig->GetString(TEXT("/Script/DatadogMod.DatadogApi"), TEXT("DatadogApiKey"), DatadogApiKey, GGameIni);
	GConfig->GetString(TEXT("/Script/DatadogMod.DatadogApi"), TEXT("DatadogSite"), DatadogSite, GGameIni);

	// Set the Datadog Host
	mHost = SiteToHost(DatadogSite);
	if (mHost.Equals("")) {
		UE_LOG(LogDatadogMod, Warning, TEXT("Unrecognized datadog site %s"), *DatadogSite);
	}
}

void UDatadogApi::Submit(TArray<FDatadogTimeseries> timeseries)
{
	// Serialize timeseries to json
	FString jsonString;

	FMetricsPayload payload;
	payload.series = timeseries;
	
	bool success = FJsonObjectConverter::UStructToJsonObjectString(payload, jsonString, 0, 0, 0, &ExportEnumAsNumber, false);
	if (!success) {
		UE_LOG(LogDatadogMod, Error, TEXT("Failed to serialize payload"));
		return;
	}

	// Compress with zlib. Zero-array means an error.
	TArray<uint8> compressedData = Compress(jsonString);
	if (compressedData.IsEmpty()) {
		return;
	}

	// Check the payload size. Compressed, it must be under 5 MB. Uncompressed, under 512kB.
	if (jsonString.Len() >= 4.5 * 1024 * 1024) {
		UE_LOG(LogDatadogMod, Warning, TEXT("Uncompressed Payload over 4.5MiB with %d metrics, max allowed is 5MiB. Recommend batching metrics."), payload.series.Num());
	}
	if (compressedData.Num() >= 500 * 1000) {
		UE_LOG(LogDatadogMod, Warning, TEXT("Compressed Payload over 500kB with %d metrics, max allowed is 512kB Recommend batching metrics."), payload.series.Num());
	}

	auto http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UDatadogApi::OnResponseReceived);

	FString url = mHost + TEXT("/api/v2/series");
	UE_LOG(LogDatadogMod, Log, TEXT("Submitting %d metrics with site %s and key %s to %s (payload size: %.1fkB)."), timeseries.Num(), *DatadogSite, *DatadogApiKey, *url, jsonString.Len() / 1024.0);
	Request->SetURL(url);
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Content-Encoding"), TEXT("deflate"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetHeader(TEXT("DD-API-KEY"), DatadogApiKey);
	Request->SetContent(compressedData);
	Request->ProcessRequest();
}

void UDatadogApi::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful) {
		UE_LOG(LogDatadogMod, Error, TEXT("Metrics submission unable to send."));
	} else {
		if (Response->GetResponseCode() != 202) {
			UE_LOG(LogDatadogMod, Error, TEXT("Metrics submission rejected: %s"), *Response->GetContentAsString());
		}
	}
}


// Compress given FString using zlib's deflate method.
// If an error occurs, Log the error and return an empty array.
TArray<uint8> UDatadogApi::Compress(const FString& text)
{
	// TODO: Need to convert before compressBound, but TCHAR_TO_UTF8 can't be stored?
	unsigned long bounds = compressBound(text.Len());
	TArray<uint8> buffer = TArray<uint8>();
	buffer.SetNum(bounds);

	int result = compress(buffer.GetData(), &bounds, (unsigned char*)TCHAR_TO_UTF8(*text), text.Len());
	buffer.SetNum(bounds);

	if (result != Z_OK) {
		if (result == Z_BUF_ERROR) {
			UE_LOG(LogDatadogMod, Error, TEXT("Failed to compress payload: not enough space in output buffer"));
		}
		else if (result == Z_MEM_ERROR) {
			UE_LOG(LogDatadogMod, Error, TEXT("Failed to compress payload: not enough memory"));
		}
		else {
			UE_LOG(LogDatadogMod, Error, TEXT("Failed to compress payload: unknown reason"));
		}
		return TArray<uint8>();
	}

	return buffer;
}
