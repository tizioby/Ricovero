#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RcvInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRcvFocusChanged, AActor*, NewFocus);

/**
 * Cerca ogni frame l'oggetto interattivo al centro dello sguardo.
 * Nessun testo di tutorial: l'HUD mostra solo un punto che si accende sul focus.
 */
UCLASS(ClassGroup = (Ricovero), meta = (BlueprintSpawnableComponent))
class RICOVERO_API URcvInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URcvInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Interagisce con l'oggetto a fuoco, se c'è. */
	UFUNCTION(BlueprintCallable, Category = "Interazione")
	bool TryInteract();

	UFUNCTION(BlueprintPure, Category = "Interazione")
	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

	UPROPERTY(BlueprintAssignable, Category = "Interazione")
	FRcvFocusChanged OnFocusChanged;

	/** Distanza massima dagli occhi (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "Interazione")
	float InteractDistance = 200.f;

	/** Raggio della traccia: un po' di tolleranza sulla mira. */
	UPROPERTY(EditDefaultsOnly, Category = "Interazione")
	float TraceRadius = 6.f;

private:
	void SetFocus(AActor* NewFocus);

	TWeakObjectPtr<AActor> FocusedActor;
};
