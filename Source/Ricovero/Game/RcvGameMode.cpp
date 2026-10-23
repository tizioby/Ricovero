#include "Game/RcvGameMode.h"

#include "Player/RcvCharacter.h"
#include "Player/RcvPlayerController.h"
#include "Ricovero.h"
#include "UI/RcvHUD.h"

ARcvGameMode::ARcvGameMode()
{
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
