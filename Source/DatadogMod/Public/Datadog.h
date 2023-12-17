#pragma once

#include "CoreMinimal.h"
#include "DatadogApi.h"
#include "DDCollector.h"
#include "Datadog.generated.h"

UCLASS()
class DATADOGMOD_API UDatadog : public UObject {
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;
	virtual void BeginPlay();
	virtual void Init(UWorld* world);
	void CollectStats();

private:
	FTimerHandle statTimerHandle;

	UPROPERTY()
	float collectionPeriod = 15.0f;

	UPROPERTY()
	UDatadogApi* datadogApi;

	UPROPERTY()
	TArray<UDDCollector*> Collectors;

	UPROPERTY()
	UWorld* mWorld;
};
