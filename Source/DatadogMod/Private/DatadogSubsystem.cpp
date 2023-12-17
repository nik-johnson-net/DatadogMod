#include "DatadogSubsystem.h"
#include "DatadogMod.h"
#include "FGGameMode.h"

void UDatadogSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	AFGGameMode* GameMode = Cast<AFGGameMode>(InWorld.GetAuthGameMode());
	if (GameMode == nullptr) {
		return;
	}
	
	if (GameMode->IsMainMenuGameMode()) {
		return;
	}

	UE_LOG(LogDatadogMod, Verbose, TEXT("DatadogSubsystem running"));
	
	mDatadog = NewObject<UDatadog>();
	mDatadog->Init(&InWorld);
	mDatadog->BeginPlay();
}

bool UDatadogSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Type::Game;
}
