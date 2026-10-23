// Comandi console di debug (solo build non Shipping). Si scrivono nella console con il tasto \.

#include "Flashlight/RcvFlashlightComponent.h"
#include "Game/RcvGameInstance.h"
#include "Game/RcvGameMode.h"
#include "Inventory/RcvInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/RcvCharacter.h"
#include "Ricovero.h"

#if !UE_BUILD_SHIPPING

namespace
{
	ARcvCharacter* GetPlayerCharacter(UWorld* World)
	{
		return World ? Cast<ARcvCharacter>(UGameplayStatics::GetPlayerPawn(World, 0)) : nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs BatteryCommand(
		TEXT("Rcv.Battery"),
		TEXT("Imposta la batteria della torcia in percentuale. Es: Rcv.Battery 15"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			ARcvCharacter* Character = GetPlayerCharacter(World);
			if (!Character || Args.Num() < 1)
			{
				return;
			}
			const float Percent = FCString::Atof(*Args[0]);
			Character->GetFlashlight()->SetBatteryRatio(Percent / 100.f);
			UE_LOG(LogRcv, Display, TEXT("Batteria torcia: %.0f%%"), Percent);
		}));

	FAutoConsoleCommandWithWorldAndArgs GiveAllCommand(
		TEXT("Rcv.GiveAll"),
		TEXT("Dà la torcia e il massimo di pile. Opzionale: id di una chiave. Es: Rcv.GiveAll Key_Ward"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			ARcvCharacter* Character = GetPlayerCharacter(World);
			if (!Character)
			{
				return;
			}
			Character->GetFlashlight()->GiveFlashlight(true);
			URcvInventoryComponent* Inventory = Character->GetInventory();
			Inventory->AddBatteries(Inventory->MaxBatteries);
			if (Args.Num() > 0)
			{
				Inventory->AddKey(FName(*Args[0]));
			}
			UE_LOG(LogRcv, Display, TEXT("Rcv.GiveAll: torcia, %d pile"), Inventory->GetBatteryCount());
		}));

	FAutoConsoleCommandWithWorldAndArgs GoToActCommand(
		TEXT("Rcv.GoToAct"),
		TEXT("Salta all'inizio di un atto (1-4) per provarne ritmo e durata. Es: Rcv.GoToAct 2"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			ARcvGameMode* GameMode = World ? World->GetAuthGameMode<ARcvGameMode>() : nullptr;
			if (GameMode && Args.Num() > 0)
			{
				GameMode->DebugGoToAct(FCString::Atoi(*Args[0]));
			}
		}));

	FAutoConsoleCommandWithWorldAndArgs TimeCommand(
		TEXT("Rcv.Time"),
		TEXT("Stampa nel log il tempo di gioco totale e per atto."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (const URcvGameInstance* GI = World ? World->GetGameInstance<URcvGameInstance>() : nullptr)
			{
				UE_LOG(LogRcvTimer, Display, TEXT("%s"), *GI->BuildTimeReport());
			}
		}));
}

#endif
