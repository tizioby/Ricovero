#pragma once

#include "CoreMinimal.h"
#include "Components/SpotLightComponent.h"
#include "RcvFlashlightComponent.generated.h"

class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRcvBatteryChanged, float, BatteryRatio);

/**
 * Torcia tenuta in mano: spot light con batteria, flicker a batteria scarica
 * e un leggero ritardo di rotazione rispetto alla camera.
 * Va attaccata alla camera del giocatore.
 */
UCLASS(ClassGroup = (Ricovero), meta = (BlueprintSpawnableComponent))
class RICOVERO_API URcvFlashlightComponent : public USpotLightComponent
{
	GENERATED_BODY()

public:
	URcvFlashlightComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Il giocatore ha raccolto la torcia: da qui può accenderla. */
	UFUNCTION(BlueprintCallable, Category = "Torcia")
	void GiveFlashlight(bool bTurnOn = true);

	UFUNCTION(BlueprintPure, Category = "Torcia")
	bool HasFlashlight() const { return bHasFlashlight; }

	UFUNCTION(BlueprintCallable, Category = "Torcia")
	void Toggle();

	UFUNCTION(BlueprintCallable, Category = "Torcia")
	void SetOn(bool bNewOn);

	UFUNCTION(BlueprintPure, Category = "Torcia")
	bool IsOn() const { return bIsOn; }

	/** Aggiunge carica (0-1) senza superare il massimo. */
	UFUNCTION(BlueprintCallable, Category = "Torcia")
	void AddCharge(float Ratio);

	UFUNCTION(BlueprintCallable, Category = "Torcia")
	void SetBatteryRatio(float Ratio);

	UFUNCTION(BlueprintPure, Category = "Torcia")
	float GetBatteryRatio() const { return BatteryLife > 0.f ? Battery / BatteryLife : 0.f; }

	UPROPERTY(BlueprintAssignable, Category = "Torcia")
	FRcvBatteryChanged OnBatteryChanged;

	/** Secondi di uso continuo con batteria piena. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Torcia|Batteria", meta = (ClampMin = "10.0"))
	float BatteryLife = 240.f;

	/** Carica che una pila aggiunge (0-1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Torcia|Batteria", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChargePerBattery = 0.6f;

	/** Sotto questa soglia la luce cala e tremola (0-1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Torcia|Batteria", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowBatteryThreshold = 0.2f;

	/** Intensità minima, come frazione della piena, appena prima che la batteria si esaurisca. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Torcia|Batteria", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinLowBatteryIntensity = 0.35f;

	/** Intensità a batteria piena, in lumen. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Torcia|Luce")
	float FullIntensityLumens = 600.f;

	/** Velocità con cui la luce insegue la camera: più bassa = più ritardo. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Torcia|Movimento")
	float RotationLagSpeed = 14.f;

	UPROPERTY(EditDefaultsOnly, Category = "Torcia|Suoni")
	TObjectPtr<USoundBase> ToggleOnSound;

	UPROPERTY(EditDefaultsOnly, Category = "Torcia|Suoni")
	TObjectPtr<USoundBase> ToggleOffSound;

	/** Click a vuoto quando si prova ad accendere con la batteria scarica. */
	UPROPERTY(EditDefaultsOnly, Category = "Torcia|Suoni")
	TObjectPtr<USoundBase> EmptyClickSound;

	/** Stato per il salvataggio. */
	float Battery = 0.f;

private:
	void UpdateIntensity(float DeltaTime);
	void PlaySound(USoundBase* Sound) const;

	bool bHasFlashlight = false;
	bool bIsOn = false;
	float FlickerTimer = 0.f;
	float FlickerValue = 1.f;
	float LastBroadcastRatio = -1.f;
};
