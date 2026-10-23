#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RcvGameMode.generated.h"

/**
 * GameMode principale. Il pawn del giocatore si legge da DefaultGame.ini (PlayerPawnClass),
 * così il Blueprint con gli asset assegnati viene usato senza riferimenti cablati nel C++.
 */
UCLASS(Config = Game)
class RICOVERO_API ARcvGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARcvGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:
	UPROPERTY(Config, EditDefaultsOnly, Category = "Classi")
	TSoftClassPtr<APawn> PlayerPawnClass;
};
