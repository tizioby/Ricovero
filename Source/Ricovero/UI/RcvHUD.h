#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Inventory/RcvInventoryComponent.h"
#include "RcvHUD.generated.h"

class SWidget;

UENUM()
enum class ERcvMenu : uint8
{
	None,
	Main,
	Pause,
	End
};

/**
 * HUD minimale senza asset: punto centrale che si accende sugli oggetti interattivi,
 * lettore dei documenti (Canvas) e menu principale/pausa/fine (Slate).
 * Nel Passo 5 lettore e menu passeranno a widget UMG con grafica vera.
 */
UCLASS()
class RICOVERO_API ARcvHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void ShowDocument(const FRcvDocument& Document);
	void HideDocument();
	bool IsShowingDocument() const { return bShowingDocument; }

	/** Mostra un menu; Extra è un testo aggiuntivo (es. i tempi nella schermata finale). */
	void ShowMenu(ERcvMenu Menu, const FString& Extra = FString());
	void HideMenu();
	ERcvMenu GetMenu() const { return CurrentMenu; }

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

	ERcvMenu CurrentMenu = ERcvMenu::None;
	TSharedPtr<SWidget> MenuWidget;
};
