#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Inventory/RcvInventoryComponent.h"
#include "RcvHUD.generated.h"

/**
 * HUD minimale disegnato su Canvas, senza asset: il punto centrale che si accende
 * sugli oggetti interattivi e il lettore dei documenti.
 * Nel Passo 5 il lettore passerà a un widget UMG con carta e grafica vera.
 */
UCLASS()
class RICOVERO_API ARcvHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	void ShowDocument(const FRcvDocument& Document);
	void HideDocument();
	bool IsShowingDocument() const { return bShowingDocument; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	float DotSize = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	float DotIdleOpacity = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	float DotFocusOpacity = 0.85f;

private:
	void DrawDot();
	void DrawDocument();
	TArray<FString> WrapText(const FString& Text, UFont* Font, float MaxWidth) const;

	bool bShowingDocument = false;
	FRcvDocument CurrentDocument;
	float DotOpacity = 0.15f;
};
