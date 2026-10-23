#include "Player/RcvPlayerController.h"

#include "Camera/PlayerCameraManager.h"

void ARcvPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = ViewPitchMin;
		PlayerCameraManager->ViewPitchMax = ViewPitchMax;
	}

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}
