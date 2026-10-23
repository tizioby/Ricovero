#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RcvInteractable.generated.h"

class ARcvCharacter;

UINTERFACE(MinimalAPI, BlueprintType)
class URcvInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Qualsiasi attore con cui il giocatore può interagire (porte, oggetti, note).
 * Implementabile sia in C++ sia in Blueprint.
 */
class RICOVERO_API IRcvInteractable
{
	GENERATED_BODY()

public:
	/** Falso per oggetti momentaneamente non interattivi (es. porta in movimento). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interazione")
	bool CanInteract(ARcvCharacter* Interactor) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interazione")
	void Interact(ARcvCharacter* Interactor);

	/** Chiamata quando lo sguardo del giocatore entra o esce dall'oggetto. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interazione")
	void SetFocused(bool bFocused);
};
