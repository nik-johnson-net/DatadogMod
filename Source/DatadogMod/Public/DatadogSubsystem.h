#pragma once

#include "CoreMinimal.h"
#include "Datadog.h"
#include "DatadogSubsystem.generated.h"

UCLASS()
class DATADOGMOD_API UDatadogSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:

	UPROPERTY()
	UDatadog* mDatadog;
};
