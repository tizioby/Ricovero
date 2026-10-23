#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RcvStaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRcvExhaustedChanged, bool, bExhausted);

/**
 * Stamina per la corsa, espressa in secondi di corsa disponibili.
 * Nessuna barra a schermo: il feedback al giocatore è il respiro (vedi ARcvCharacter).
 */
UCLASS(ClassGroup = (Ricovero), meta = (BlueprintSpawnableComponent))
class RICOVERO_API URcvStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URcvStaminaComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Il proprietario sta correndo in questo frame: consuma stamina. */
	void SetDraining(bool bInDraining) { bDraining = bInDraining; }

	/** Vero se si può iniziare (o continuare) a correre. */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool CanSprint() const;

	/** 0 = vuota, 1 = piena. */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetStaminaRatio() const { return MaxStamina > 0.f ? Stamina / MaxStamina : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool IsExhausted() const { return bExhausted; }

	UPROPERTY(BlueprintAssignable, Category = "Stamina")
	FRcvExhaustedChanged OnExhaustedChanged;

	/** Secondi di corsa continua con stamina piena. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.5"))
	float MaxStamina = 6.f;

	/** Attesa dopo aver smesso di correre prima che la stamina ricominci a salire. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0"))
	float RegenDelay = 1.5f;

	/** Secondi per ricaricare da vuota a piena. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.5"))
	float FullRegenTime = 5.f;

	/** Dopo l'esaurimento non si corre finché la stamina non torna sopra questa soglia (0-1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RecoverThreshold = 0.35f;

private:
	float Stamina = 0.f;
	float TimeSinceDrain = 0.f;
	bool bDraining = false;
	bool bExhausted = false;

	void SetExhausted(bool bNew);
};
