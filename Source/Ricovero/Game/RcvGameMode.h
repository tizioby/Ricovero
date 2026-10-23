#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RcvGameMode.generated.h"

class URcvGameInstance;
class ARcvCharacter;

/**
 * GameMode principale: flusso menu -> partita -> fine, checkpoint per atto, timer di sessione.
 * Il pawn del giocatore si legge da DefaultGame.ini (PlayerPawnClass),
 * così il Blueprint con gli asset assegnati viene usato senza riferimenti cablati nel C++.
 */
UCLASS(Config = Game)
class RICOVERO_API ARcvGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARcvGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void Tick(float DeltaSeconds) override;

	// --- Chiamate dai menu ---
	void StartNewGame();
	void ContinueFromCheckpoint();
	void RestartFromCheckpoint();
	void ReturnToMainMenu();
	void QuitGame();
	bool HasCheckpoint() const;

	// --- Chiamate dal livello ---
	/** Il giocatore entra in un atto: log del tempo e salvataggio automatico. */
	void ReachAct(int32 Act);
	void EndGame();

	/** Teletrasporta all'inizio dell'atto (debug: Rcv.GoToAct). */
	void DebugGoToAct(int32 Act);

	URcvGameInstance* GetRcvGameInstance() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Config, EditDefaultsOnly, Category = "Classi")
	TSoftClassPtr<APawn> PlayerPawnClass;

	/** Secondi di dissolvenza dal nero al risveglio. */
	UPROPERTY(EditDefaultsOnly, Category = "Atto 1")
	float WakeUpFadeTime = 5.f;

private:
	void HandleStartMode();
	void BeginRun(int32 Act, bool bFadeIn);
	void SaveCheckpoint();
	bool ApplyCheckpoint();
	void TeleportToAct(int32 Act);
	ARcvCharacter* GetPlayerCharacter() const;
	void ReloadLevel();
};
