#include "Player/RcvPlayerController.h"

#include "Camera/PlayerCameraManager.h"
#include "UI/RcvHUD.h"

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

void ARcvPlayerController::ShowDocument(const FRcvDocument& Document)
{
	if (ARcvHUD* RcvHUD = GetHUD<ARcvHUD>())
	{
		RcvHUD->ShowDocument(Document);
		SetIgnoreMoveInput(true);
		SetIgnoreLookInput(true);
	}
}

void ARcvPlayerController::CloseDocument()
{
	if (ARcvHUD* RcvHUD = GetHUD<ARcvHUD>())
	{
		RcvHUD->HideDocument();
	}
	ResetIgnoreMoveInput();
	ResetIgnoreLookInput();
}

bool ARcvPlayerController::IsReadingDocument() const
{
	const ARcvHUD* RcvHUD = GetHUD<ARcvHUD>();
	return RcvHUD && RcvHUD->IsShowingDocument();
}
