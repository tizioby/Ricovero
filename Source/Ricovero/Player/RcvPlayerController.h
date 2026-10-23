#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RcvPlayerController.generated.h"

struct FRcvDocument;

UCLASS()
class RICOVERO_API ARcvPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** Mostra un documento a schermo intero e blocca movimento e sguardo finché non si chiude. */
	void ShowDocument(const FRcvDocument& Document);
	void CloseDocument();
	bool IsReadingDocument() const;

protected:
	virtual void BeginPlay() override;

	/** Limiti dello sguardo verticale (gradi). */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float ViewPitchMin = -80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float ViewPitchMax = 80.f;
};
