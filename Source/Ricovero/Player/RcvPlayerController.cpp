#include "Player/RcvPlayerController.h"

#include "Camera/PlayerCameraManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

ARcvPlayerController::ARcvPlayerController()
{
	// Serve per ricevere il tasto pausa anche quando il gioco è fermo.
	bShouldPerformFullTickWhenPaused = true;
}

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

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (PauseMappingContext)
		{
			Subsystem->AddMappingContext(PauseMappingContext, 10);
		}
	}
}

void ARcvPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	PauseAction = NewObject<UInputAction>(this, TEXT("IA_Pause_Runtime"));
	PauseAction->ValueType = EInputActionValueType::Boolean;
	PauseAction->bTriggerWhenPaused = true;

	PauseMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Pause_Runtime"));
	PauseMappingContext->MapKey(PauseAction, EKeys::Escape);
	// P in aggiunta a Esc: nell'editor Esc ferma il Play.
	PauseMappingContext->MapKey(PauseAction, EKeys::P);
	PauseMappingContext->MapKey(PauseAction, EKeys::Gamepad_Special_Right);

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
	{
		Input->BindAction(PauseAction, ETriggerEvent::Started, this, &ARcvPlayerController::TogglePause);
	}
}

void ARcvPlayerController::TogglePause()
{
	const ARcvHUD* RcvHUD = GetHUD<ARcvHUD>();
	if (!RcvHUD)
	{
		return;
	}
	switch (RcvHUD->GetMenu())
	{
	case ERcvMenu::None:
		if (IsReadingDocument())
		{
			CloseDocument();
		}
		else
		{
			ShowMenu(ERcvMenu::Pause);
		}
		break;
	case ERcvMenu::Pause:
		HideMenu();
		break;
	default:
		// Menu principale e schermata finale si chiudono solo dai pulsanti.
		break;
	}
}

void ARcvPlayerController::ShowMenu(ERcvMenu Menu, const FString& Extra)
{
	if (ARcvHUD* RcvHUD = GetHUD<ARcvHUD>())
	{
		RcvHUD->ShowMenu(Menu, Extra);
	}
}

void ARcvPlayerController::HideMenu()
{
	if (ARcvHUD* RcvHUD = GetHUD<ARcvHUD>())
	{
		RcvHUD->HideMenu();
	}
	SetPause(false);
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
