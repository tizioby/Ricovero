#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Inventory/RcvInventoryComponent.h"
#include "RcvSaveGame.generated.h"

/** Checkpoint automatico: scritto all'inizio di ogni atto. */
UCLASS()
class RICOVERO_API URcvSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	static const FString SlotName;

	UPROPERTY()
	int32 Act = 1;

	UPROPERTY()
	float PlayTime = 0.f;

	UPROPERTY()
	TArray<float> ActStartTimes;

	UPROPERTY()
	bool bHasFlashlight = false;

	UPROPERTY()
	float BatteryRatio = 1.f;

	UPROPERTY()
	int32 Batteries = 0;

	UPROPERTY()
	TArray<FName> Keys;

	UPROPERTY()
	TArray<FRcvDocument> Documents;
};
