#include "Game/RcvActTrigger.h"

#include "Components/BoxComponent.h"
#include "Game/RcvGameMode.h"
#include "GameFramework/Pawn.h"

ARcvActTrigger::ARcvActTrigger()
{
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);
	Box->SetBoxExtent(FVector(50.f, 150.f, 150.f));
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetGenerateOverlapEvents(true);
	Box->ShapeColor = FColor::Green;
	Box->OnComponentBeginOverlap.AddDynamic(this, &ARcvActTrigger::OnOverlap);
}

void ARcvActTrigger::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (bTriggered || !Pawn || !Pawn->IsPlayerControlled())
	{
		return;
	}
	ARcvGameMode* GameMode = GetWorld()->GetAuthGameMode<ARcvGameMode>();
	if (!GameMode)
	{
		return;
	}

	bTriggered = true;
	if (bEndsGame)
	{
		GameMode->EndGame();
	}
	else
	{
		GameMode->ReachAct(Act);
	}
}
