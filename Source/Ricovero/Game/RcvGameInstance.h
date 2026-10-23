#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RcvGameInstance.generated.h"

/** Cosa fare quando la mappa di gioco si carica. */
UENUM()
enum class ERcvStartMode : uint8
{
	MainMenu,
	NewGame,
	Continue
};

RICOVERO_API DECLARE_LOG_CATEGORY_EXTERN(LogRcvTimer, Log, All);

/**
 * Sopravvive ai caricamenti di livello: modalità di avvio, timer di sessione e tempi per atto.
 * Il timer conta solo il tempo di gioco (non menu e pausa).
 */
UCLASS()
class RICOVERO_API URcvGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	static constexpr int32 NumActs = 4;

	ERcvStartMode StartMode = ERcvStartMode::MainMenu;

	bool bRunActive = false;
	float PlayTime = 0.f;
	int32 CurrentAct = 1;
	/** Tempo di gioco all'ingresso di ogni atto (indice 0 = Atto 1), -1 se non raggiunto. */
	TArray<float> ActStartTimes;

	void ResetRun();
	void EnterAct(int32 Act);
	FString BuildTimeReport() const;

	static FString FormatTime(float Seconds);
};
