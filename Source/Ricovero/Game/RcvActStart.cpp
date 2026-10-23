#include "Game/RcvActStart.h"

#include "Components/ArrowComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

ARcvActStart::ARcvActStart()
{
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

#if WITH_EDITORONLY_DATA
	Arrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (Arrow)
	{
		Arrow->SetupAttachment(GetRootComponent());
		Arrow->ArrowColor = FColor::Cyan;
		Arrow->ArrowSize = 2.f;
	}
#endif
}

ARcvActStart* ARcvActStart::Find(const UObject* WorldContext, int32 Act)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (!World)
	{
		return nullptr;
	}
	for (TActorIterator<ARcvActStart> It(World); It; ++It)
	{
		if (It->Act == Act)
		{
			return *It;
		}
	}
	return nullptr;
}
