#include "Game/RcvGameMode.h"

#include "Camera/PlayerCameraManager.h"
#include "Flashlight/RcvFlashlightComponent.h"
#include "Game/RcvActStart.h"
#include "Game/RcvGameInstance.h"
#include "Game/RcvSaveGame.h"
#include "Inventory/RcvInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/RcvCharacter.h"
#include "Player/RcvPlayerController.h"
#include "Ricovero.h"
#include "TimerManager.h"
#include "UI/RcvHUD.h"

ARcvGameMode::ARcvGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = ARcvCharacter::StaticClass();
	PlayerControllerClass = ARcvPlayerController::StaticClass();
	HUDClass = ARcvHUD::StaticClass();
}

void ARcvGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	if (!PlayerPawnClass.IsNull())
	{
		if (UClass* Loaded = PlayerPawnClass.LoadSynchronous())
		{
			DefaultPawnClass = Loaded;
		}
		else
		{
			UE_LOG(LogRcv, Warning, TEXT("PlayerPawnClass %s non trovato, uso ARcvCharacter senza asset."), *PlayerPawnClass.ToString());
		}
	}
}

void ARcvGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Al primo frame il giocatore è già stato creato e posseduto.
	GetWorldTimerManager().SetTimerForNextTick(this, &ARcvGameMode::HandleStartMode);
}

URcvGameInstance* ARcvGameMode::GetRcvGameInstance() const
{
	return GetGameInstance<URcvGameInstance>();
}

ARcvCharacter* ARcvGameMode::GetPlayerCharacter() const
{
	return Cast<ARcvCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
}

void ARcvGameMode::HandleStartMode()
{
	URcvGameInstance* GI = GetRcvGameInstance();
	ARcvPlayerController* PC = Cast<ARcvPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (!GI || !PC)
	{
		UE_LOG(LogRcv, Warning, TEXT("GameInstance o PlayerController non di Ricovero: controlla DefaultEngine.ini."));
		return;
	}

	// Mappe senza punti di partenza (es. mappa di prova): si gioca subito, senza menu.
	if (!ARcvActStart::Find(this, 1))
	{
		BeginRun(1, false);
		return;
	}

	switch (GI->StartMode)
	{
	case ERcvStartMode::MainMenu:
		PC->ShowMenu(ERcvMenu::Main);
		break;
	case ERcvStartMode::NewGame:
		StartNewGame();
		break;
	case ERcvStartMode::Continue:
		if (!ApplyCheckpoint())
		{
			StartNewGame();
		}
		break;
	}
	GI->StartMode = ERcvStartMode::MainMenu;
}

void ARcvGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Il GameMode non ticka in pausa, quindi il timer conta solo il tempo di gioco.
	if (URcvGameInstance* GI = GetRcvGameInstance(); GI && GI->bRunActive)
	{
		GI->PlayTime += DeltaSeconds;
	}
}

void ARcvGameMode::StartNewGame()
{
	URcvGameInstance* GI = GetRcvGameInstance();
	GI->ResetRun();
	UGameplayStatics::DeleteGameInSlot(URcvSaveGame::SlotName, 0);
	TeleportToAct(1);
	BeginRun(1, true);
	SaveCheckpoint();
}

void ARcvGameMode::ContinueFromCheckpoint()
{
	// Ricarica il livello così eventi e oggetti tornano allo stato iniziale, poi applica il salvataggio.
	GetRcvGameInstance()->StartMode = ERcvStartMode::Continue;
	ReloadLevel();
}

void ARcvGameMode::RestartFromCheckpoint()
{
	ContinueFromCheckpoint();
}

void ARcvGameMode::ReturnToMainMenu()
{
	URcvGameInstance* GI = GetRcvGameInstance();
	GI->bRunActive = false;
	GI->StartMode = ERcvStartMode::MainMenu;
	ReloadLevel();
}

void ARcvGameMode::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, UGameplayStatics::GetPlayerController(this, 0), EQuitPreference::Quit, false);
}

bool ARcvGameMode::HasCheckpoint() const
{
	return UGameplayStatics::DoesSaveGameExist(URcvSaveGame::SlotName, 0);
}

void ARcvGameMode::BeginRun(int32 Act, bool bFadeIn)
{
	URcvGameInstance* GI = GetRcvGameInstance();
	if (GI->ActStartTimes.Num() != URcvGameInstance::NumActs)
	{
		GI->ResetRun();
	}
	GI->bRunActive = true;
	GI->EnterAct(Act);

	if (ARcvPlayerController* PC = Cast<ARcvPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->HideMenu();
		if (bFadeIn && PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, WakeUpFadeTime, FLinearColor::Black, false, true);
		}
	}
}

void ARcvGameMode::ReachAct(int32 Act)
{
	URcvGameInstance* GI = GetRcvGameInstance();
	if (!GI || Act <= GI->CurrentAct)
	{
		return;
	}
	GI->EnterAct(Act);
	SaveCheckpoint();
}

void ARcvGameMode::EndGame()
{
	URcvGameInstance* GI = GetRcvGameInstance();
	GI->bRunActive = false;
	UE_LOG(LogRcvTimer, Display, TEXT("Fine partita.\n%s"), *GI->BuildTimeReport());
	UGameplayStatics::DeleteGameInSlot(URcvSaveGame::SlotName, 0);

	if (ARcvPlayerController* PC = Cast<ARcvPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->ShowMenu(ERcvMenu::End, GI->BuildTimeReport());
	}
}

void ARcvGameMode::DebugGoToAct(int32 Act)
{
	URcvGameInstance* GI = GetRcvGameInstance();
	if (!GI->bRunActive)
	{
		BeginRun(Act, false);
	}
	TeleportToAct(Act);
	if (Act > GI->CurrentAct)
	{
		GI->EnterAct(Act);
	}
	else
	{
		GI->CurrentAct = Act;
	}
	UE_LOG(LogRcvTimer, Display, TEXT("[Debug] Salto all'Atto %d (tempo %s)"), Act, *URcvGameInstance::FormatTime(GI->PlayTime));
}

void ARcvGameMode::TeleportToAct(int32 Act)
{
	ARcvCharacter* Character = GetPlayerCharacter();
	const ARcvActStart* Start = ARcvActStart::Find(this, Act);
	if (!Character || !Start)
	{
		UE_LOG(LogRcv, Warning, TEXT("Nessun ARcvActStart per l'Atto %d nella mappa."), Act);
		return;
	}
	Character->TeleportTo(Start->GetActorLocation(), Start->GetActorRotation());
	if (AController* Controller = Character->GetController())
	{
		Controller->SetControlRotation(Start->GetActorRotation());
	}
}

void ARcvGameMode::SaveCheckpoint()
{
	const URcvGameInstance* GI = GetRcvGameInstance();
	const ARcvCharacter* Character = GetPlayerCharacter();
	URcvSaveGame* Save = Cast<URcvSaveGame>(UGameplayStatics::CreateSaveGameObject(URcvSaveGame::StaticClass()));
	if (!GI || !Character || !Save)
	{
		return;
	}

	Save->Act = GI->CurrentAct;
	Save->PlayTime = GI->PlayTime;
	Save->ActStartTimes = GI->ActStartTimes;
	Save->bHasFlashlight = Character->GetFlashlight()->HasFlashlight();
	Save->BatteryRatio = Character->GetFlashlight()->GetBatteryRatio();

	const URcvInventoryComponent* Inventory = Character->GetInventory();
	Save->Batteries = Inventory->Batteries;
	Save->Keys = Inventory->Keys.Array();
	Save->Documents = Inventory->Documents;

	UGameplayStatics::SaveGameToSlot(Save, URcvSaveGame::SlotName, 0);
	UE_LOG(LogRcvTimer, Display, TEXT("Checkpoint salvato: Atto %d, %s"), Save->Act, *URcvGameInstance::FormatTime(Save->PlayTime));
}

bool ARcvGameMode::ApplyCheckpoint()
{
	const URcvSaveGame* Save = Cast<URcvSaveGame>(UGameplayStatics::LoadGameFromSlot(URcvSaveGame::SlotName, 0));
	ARcvCharacter* Character = GetPlayerCharacter();
	URcvGameInstance* GI = GetRcvGameInstance();
	if (!Save || !Character || !GI)
	{
		return false;
	}

	GI->PlayTime = Save->PlayTime;
	GI->ActStartTimes = Save->ActStartTimes;
	GI->CurrentAct = Save->Act;

	if (Save->bHasFlashlight)
	{
		Character->GetFlashlight()->GiveFlashlight(false);
	}
	Character->GetFlashlight()->SetBatteryRatio(Save->BatteryRatio);

	URcvInventoryComponent* Inventory = Character->GetInventory();
	Inventory->Batteries = Save->Batteries;
	Inventory->Keys = TSet<FName>(Save->Keys);
	Inventory->Documents = Save->Documents;
	Inventory->OnInventoryChanged.Broadcast();

	TeleportToAct(Save->Act);
	BeginRun(Save->Act, true);
	UE_LOG(LogRcvTimer, Display, TEXT("Ripreso dal checkpoint: Atto %d, %s"), Save->Act, *URcvGameInstance::FormatTime(Save->PlayTime));
	return true;
}

void ARcvGameMode::ReloadLevel()
{
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}
