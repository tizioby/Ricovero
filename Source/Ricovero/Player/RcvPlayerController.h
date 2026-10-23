#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/RcvHUD.h"
#include "RcvPlayerController.generated.h"

struct FRcvDocument;
class UInputAction;
class UInputMappingContext;

UCLASS()
class RICOVERO_API ARcvPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARcvPlayerController();

	/** Mostra un documento a schermo intero e blocca movimento e sguardo finché non si chiude. */
	void ShowDocument(const FRcvDocument& Document);
	void CloseDocument();
	bool IsReadingDocument() const;

	/** Mette in pausa e mostra il menu. */
	void ShowMenu(ERcvMenu Menu, const FString& Extra = FString());
	/** Chiude il menu e riprende il gioco. */
	void HideMenu();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Limiti dello sguardo verticale (gradi). */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float ViewPitchMin = -80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float ViewPitchMax = 80.f;

private:
	void TogglePause();

	/** Azione pausa creata in C++: deve funzionare anche a gioco in pausa. */
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> PauseAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> PauseMappingContext;
};
