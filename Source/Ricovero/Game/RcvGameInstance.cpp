#include "Game/RcvGameInstance.h"

DEFINE_LOG_CATEGORY(LogRcvTimer);

void URcvGameInstance::ResetRun()
{
	bRunActive = false;
	PlayTime = 0.f;
	CurrentAct = 1;
	ActStartTimes.Init(-1.f, NumActs);
}

void URcvGameInstance::EnterAct(int32 Act)
{
	if (ActStartTimes.Num() != NumActs)
	{
		ActStartTimes.Init(-1.f, NumActs);
	}
	Act = FMath::Clamp(Act, 1, NumActs);

	const int32 Previous = CurrentAct;
	CurrentAct = Act;
	if (ActStartTimes[Act - 1] < 0.f)
	{
		ActStartTimes[Act - 1] = PlayTime;
	}

	if (Act > Previous && ActStartTimes[Previous - 1] >= 0.f)
	{
		UE_LOG(LogRcvTimer, Display, TEXT("Atto %d raggiunto a %s (Atto %d durato %s)"),
			Act, *FormatTime(PlayTime), Previous, *FormatTime(PlayTime - ActStartTimes[Previous - 1]));
	}
	else
	{
		UE_LOG(LogRcvTimer, Display, TEXT("Atto %d iniziato a %s"), Act, *FormatTime(PlayTime));
	}
}

FString URcvGameInstance::BuildTimeReport() const
{
	FString Report = FString::Printf(TEXT("Tempo totale: %s"), *FormatTime(PlayTime));
	for (int32 i = 0; i < ActStartTimes.Num(); ++i)
	{
		if (ActStartTimes[i] < 0.f)
		{
			continue;
		}
		float End = PlayTime;
		for (int32 j = i + 1; j < ActStartTimes.Num(); ++j)
		{
			if (ActStartTimes[j] >= 0.f)
			{
				End = ActStartTimes[j];
				break;
			}
		}
		Report += FString::Printf(TEXT("\nAtto %d: %s"), i + 1, *FormatTime(End - ActStartTimes[i]));
	}
	return Report;
}

FString URcvGameInstance::FormatTime(float Seconds)
{
	const int32 Total = FMath::Max(0, FMath::FloorToInt(Seconds));
	return FString::Printf(TEXT("%d:%02d"), Total / 60, Total % 60);
}
