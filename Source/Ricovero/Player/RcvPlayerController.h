#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RcvPlayerController.generated.h"

UCLASS()
class RICOVERO_API ARcvPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	/** Limiti dello sguardo verticale (gradi). */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float ViewPitchMin = -80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float ViewPitchMax = 80.f;
};
