#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RcvNeonLight.generated.h"

class URectLightComponent;
class UStaticMeshComponent;
class UAudioComponent;
class USoundBase;

UENUM(BlueprintType)
enum class ERcvNeonState : uint8
{
	On,
	Off,
	/** Tremola a scatti in modo irregolare. */
	Flicker
};

/**
 * Plafoniera al neon: rect light rivolta verso il basso, ronzio in loop, stato acceso/spento/tremolante.
 * Usata dagli eventi scriptati (luci che saltano).
 */
UCLASS()
class RICOVERO_API ARcvNeonLight : public AActor
{
	GENERATED_BODY()

public:
	ARcvNeonLight();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Neon")
	void SetState(ERcvNeonState NewState, bool bPlaySound = true);

	UFUNCTION(BlueprintPure, Category = "Neon")
	ERcvNeonState GetState() const { return State; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componenti")
	TObjectPtr<UStaticMeshComponent> Fixture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componenti")
	TObjectPtr<URectLightComponent> Light;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componenti")
	TObjectPtr<UAudioComponent> Hum;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neon")
	ERcvNeonState InitialState = ERcvNeonState::On;

	/** Intensità quando è accesa, in lumen. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Neon")
	float OnIntensityLumens = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Neon|Suoni")
	TObjectPtr<USoundBase> HumLoop;

	/** Scatto/scoppio quando si spegne di colpo. */
	UPROPERTY(EditAnywhere, Category = "Neon|Suoni")
	TObjectPtr<USoundBase> OffSound;

	UPROPERTY(EditAnywhere, Category = "Neon|Suoni")
	TObjectPtr<USoundBase> OnSound;

private:
	void ApplyLit(bool bLit);

	ERcvNeonState State = ERcvNeonState::On;
	float FlickerTimer = 0.f;
	bool bFlickerLit = true;
};
