#include "UI/RcvHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Interaction/RcvInteractionComponent.h"
#include "Player/RcvCharacter.h"

void ARcvHUD::DrawHUD()
{
	Super::DrawHUD();

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
