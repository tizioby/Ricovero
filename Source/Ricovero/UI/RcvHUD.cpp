#include "UI/RcvHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/GameViewportClient.h"
#include "Game/RcvGameMode.h"
#include "Interaction/RcvInteractionComponent.h"
#include "Player/RcvCharacter.h"
#include "Player/RcvPlayerController.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void ARcvHUD::DrawHUD()
{
	Super::DrawHUD();

	if (CurrentMenu != ERcvMenu::None)
	{
		return;
	}
	if (bShowingDocument)
	{
		DrawDocument();
	}
	else
	{
		DrawDot();
	}
}

void ARcvHUD::ShowDocument(const FRcvDocument& Document)
{
	CurrentDocument = Document;
	bShowingDocument = true;
}

void ARcvHUD::HideDocument()
{
	bShowingDocument = false;
}

void ARcvHUD::DrawDot()
{
	const ARcvCharacter* Character = Cast<ARcvCharacter>(GetOwningPawn());
	const bool bFocus = Character && Character->GetInteraction()->GetFocusedActor() != nullptr;
	const float Target = bFocus ? DotFocusOpacity : DotIdleOpacity;
	DotOpacity = FMath::FInterpTo(DotOpacity, Target, GetWorld()->GetDeltaSeconds(), 12.f);

	const float X = Canvas->ClipX * 0.5f - DotSize * 0.5f;
	const float Y = Canvas->ClipY * 0.5f - DotSize * 0.5f;
	DrawRect(FLinearColor(1.f, 1.f, 1.f, DotOpacity), X, Y, DotSize, DotSize);
}

void ARcvHUD::DrawDocument()
{
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.85f), 0.f, 0.f, Canvas->ClipX, Canvas->ClipY);

	UFont* TitleFont = GEngine->GetLargeFont();
	UFont* BodyFont = GEngine->GetMediumFont();
	const float Scale = FMath::Max(1.f, Canvas->ClipY / 1080.f);
	const float Width = FMath::Min(Canvas->ClipX * 0.6f, 900.f * Scale);
	const float Left = (Canvas->ClipX - Width) * 0.5f;
	float Top = Canvas->ClipY * 0.15f;

	const FLinearColor Paper(0.86f, 0.84f, 0.78f, 1.f);
	DrawText(CurrentDocument.Title.ToString(), Paper, Left, Top, TitleFont, Scale * 1.3f);
	Top += TitleFont->GetMaxCharHeight() * Scale * 2.2f;

	const float LineHeight = BodyFont->GetMaxCharHeight() * Scale * 1.35f;
	for (const FString& Line : WrapText(CurrentDocument.Body.ToString(), BodyFont, Width / Scale))
	{
		DrawText(Line, Paper, Left, Top, BodyFont, Scale);
		Top += LineHeight;
	}

	DrawText(TEXT("[E]"), FLinearColor(1.f, 1.f, 1.f, 0.4f), Left, Canvas->ClipY * 0.9f, BodyFont, Scale);
}

TArray<FString> ARcvHUD::WrapText(const FString& Text, UFont* Font, float MaxWidth) const
{
	TArray<FString> Lines;
	TArray<FString> Paragraphs;
	Text.ParseIntoArray(Paragraphs, TEXT("\n"), false);

	for (const FString& Paragraph : Paragraphs)
	{
		TArray<FString> Words;
		Paragraph.ParseIntoArrayWS(Words);
		FString Current;
		for (const FString& Word : Words)
		{
			const FString Candidate = Current.IsEmpty() ? Word : Current + TEXT(" ") + Word;
			float W = 0.f, H = 0.f;
			GetTextSize(Candidate, W, H, Font);
			if (W > MaxWidth && !Current.IsEmpty())
			{
				Lines.Add(Current);
				Current = Word;
			}
			else
			{
				Current = Candidate;
			}
		}
		Lines.Add(Current);
	}
	return Lines;
}

namespace
{
	enum class EMenuAction : uint8
	{
		NewGame,
		Continue,
		Resume,
		RestartCheckpoint,
		MainMenu,
		Quit
	};
}

void ARcvHUD::ShowMenu(ERcvMenu Menu, const FString& Extra)
{
	HideMenu();
	if (Menu == ERcvMenu::None || !GEngine || !GEngine->GameViewport)
	{
		return;
	}
	CurrentMenu = Menu;

	ARcvGameMode* GameMode = GetWorld()->GetAuthGameMode<ARcvGameMode>();
	TWeakObjectPtr<ARcvHUD> WeakThis(this);

	auto RunAction = [WeakThis](EMenuAction Action)
	{
		ARcvHUD* HUD = WeakThis.Get();
		ARcvGameMode* GM = HUD ? HUD->GetWorld()->GetAuthGameMode<ARcvGameMode>() : nullptr;
		if (!GM)
		{
			return;
		}
		switch (Action)
		{
		case EMenuAction::NewGame:			GM->StartNewGame(); break;
		case EMenuAction::Continue:			GM->ContinueFromCheckpoint(); break;
		case EMenuAction::RestartCheckpoint:	GM->RestartFromCheckpoint(); break;
		case EMenuAction::MainMenu:			GM->ReturnToMainMenu(); break;
		case EMenuAction::Quit:				GM->QuitGame(); break;
		case EMenuAction::Resume:
			if (ARcvPlayerController* PC = Cast<ARcvPlayerController>(HUD->GetOwningPlayerController()))
			{
				PC->HideMenu();
			}
			break;
		}
	};

	FText Title;
	TArray<TPair<FText, EMenuAction>> Buttons;
	switch (Menu)
	{
	case ERcvMenu::Main:
		Title = FText::FromString(TEXT("RICOVERO"));
		Buttons.Add({ FText::FromString(TEXT("Nuova partita")), EMenuAction::NewGame });
		if (GameMode && GameMode->HasCheckpoint())
		{
			Buttons.Add({ FText::FromString(TEXT("Continua")), EMenuAction::Continue });
		}
		Buttons.Add({ FText::FromString(TEXT("Esci")), EMenuAction::Quit });
		break;
	case ERcvMenu::Pause:
		Title = FText::FromString(TEXT("Pausa"));
		Buttons.Add({ FText::FromString(TEXT("Riprendi")), EMenuAction::Resume });
		Buttons.Add({ FText::FromString(TEXT("Ricomincia dall'ultimo checkpoint")), EMenuAction::RestartCheckpoint });
		Buttons.Add({ FText::FromString(TEXT("Menu principale")), EMenuAction::MainMenu });
		break;
	case ERcvMenu::End:
		Title = FText::FromString(TEXT("Fine"));
		Buttons.Add({ FText::FromString(TEXT("Menu principale")), EMenuAction::MainMenu });
		Buttons.Add({ FText::FromString(TEXT("Esci")), EMenuAction::Quit });
		break;
	default:
		break;
	}

	const FLinearColor TextColor(0.86f, 0.84f, 0.78f, 1.f);
	TSharedRef<SVerticalBox> Column = SNew(SVerticalBox);

	Column->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 48.f)
	[
		SNew(STextBlock)
		.Text(Title)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 56))
		.ColorAndOpacity(TextColor)
	];

	if (!Extra.IsEmpty())
	{
		Column->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 40.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Extra))
			.Justification(ETextJustify::Center)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 22))
			.ColorAndOpacity(TextColor)
		];
	}

	TSharedPtr<SButton> FirstButton;
	for (const TPair<FText, EMenuAction>& Entry : Buttons)
	{
		const EMenuAction Action = Entry.Value;
		TSharedRef<SButton> Button = SNew(SButton)
			.HAlign(HAlign_Center)
			.ButtonColorAndOpacity(FLinearColor(0.05f, 0.05f, 0.05f, 0.9f))
			.OnClicked_Lambda([RunAction, Action]()
			{
				RunAction(Action);
				return FReply::Handled();
			})
			[
				SNew(STextBlock)
				.Text(Entry.Key)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 26))
				.ColorAndOpacity(TextColor)
			];
		if (!FirstButton.IsValid())
		{
			FirstButton = Button;
		}
		Column->AddSlot().AutoHeight().Padding(0.f, 6.f)
		[
			SNew(SBox).WidthOverride(520.f)[ Button ]
		];
	}

	MenuWidget = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, Menu == ERcvMenu::Pause ? 0.7f : 0.88f))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			Column
		];

	GEngine->GameViewport->AddViewportWidgetContent(MenuWidget.ToSharedRef(), 100);

	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->SetPause(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		if (FirstButton.IsValid())
		{
			InputMode.SetWidgetToFocus(FirstButton);
		}
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

void ARcvHUD::HideMenu()
{
	if (MenuWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidget.ToSharedRef());
	}
	MenuWidget.Reset();
	CurrentMenu = ERcvMenu::None;
}

void ARcvHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	HideMenu();
	Super::EndPlay(EndPlayReason);
}
