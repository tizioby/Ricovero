#include "Narrative/RcvScriptedTrigger.h"

#include "Components/BoxComponent.h"
#include "Components/LightComponent.h"
#include "GameFramework/Pawn.h"
#include "Interaction/RcvDoor.h"
#include "Kismet/GameplayStatics.h"
#include "Narrative/RcvNeonLight.h"
#include "TimerManager.h"

ARcvScriptedTrigger::ARcvScriptedTrigger()
{
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);
	Box->SetBoxExtent(FVector(50.f, 150.f, 150.f));
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetGenerateOverlapEvents(true);
	Box->ShapeColor = FColor::Orange;
	Box->OnComponentBeginOverlap.AddDynamic(this, &ARcvScriptedTrigger::OnOverlap);
}

void ARcvScriptedTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (bHideShowTargetsAtStart)
	{
		for (const FRcvScriptStep& Step : Steps)
		{
			if (Step.Type != ERcvStepType::ShowActors)
			{
				continue;
			}
			for (AActor* Target : Step.Targets)
			{
				if (Target)
				{
					Target->SetActorHiddenInGame(true);
					Target->SetActorEnableCollision(false);
				}
			}
		}
	}
}

void ARcvScriptedTrigger::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn && Pawn->IsPlayerControlled())
	{
		Run();
	}
}

void ARcvScriptedTrigger::Run()
{
	if (bOnlyOnce && bHasRun)
	{
		return;
	}
	bHasRun = true;

	float Time = 0.f;
	for (const FRcvScriptStep& Step : Steps)
	{
		Time += Step.Delay;

		TArray<AActor*> Targets;
		for (AActor* Target : Step.Targets)
		{
			Targets.Add(Target);
		}
		if (Targets.IsEmpty())
		{
			// PlaySound senza bersagli suona sul trigger.
			Targets.Add(this);
		}

		for (int32 i = 0; i < Targets.Num(); ++i)
		{
			const float At = Time + Step.Stagger * i;
			TWeakObjectPtr<AActor> WeakTarget = Targets[i];
			const FRcvScriptStep StepCopy = Step;
			auto Execute = [this, StepCopy, WeakTarget]()
			{
				if (AActor* Target = WeakTarget.Get())
				{
					ExecuteOnTarget(StepCopy, Target);
				}
			};

			if (At <= 0.f)
			{
				Execute();
			}
			else
			{
				FTimerHandle Handle;
				GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, Execute), At, false);
			}
		}
		Time += Step.Stagger * FMath::Max(0, Targets.Num() - 1);
	}
}

void ARcvScriptedTrigger::ExecuteOnTarget(const FRcvScriptStep& Step, AActor* Target)
{
	switch (Step.Type)
	{
	case ERcvStepType::LightsOff:
	case ERcvStepType::LightsOn:
	case ERcvStepType::LightsFlicker:
	{
		if (ARcvNeonLight* Neon = Cast<ARcvNeonLight>(Target))
		{
			const ERcvNeonState State = Step.Type == ERcvStepType::LightsOff ? ERcvNeonState::Off
				: (Step.Type == ERcvStepType::LightsOn ? ERcvNeonState::On : ERcvNeonState::Flicker);
			Neon->SetState(State);
		}
		else
		{
			TArray<ULightComponent*> Lights;
			Target->GetComponents(Lights);
			for (ULightComponent* Light : Lights)
			{
				Light->SetVisibility(Step.Type != ERcvStepType::LightsOff);
			}
		}
		if (Step.Sound && Target != this)
		{
			UGameplayStatics::PlaySoundAtLocation(this, Step.Sound, Target->GetActorLocation());
		}
		break;
	}
	case ERcvStepType::DoorSlam:
		if (ARcvDoor* Door = Cast<ARcvDoor>(Target))
		{
			Door->Slam(Step.bLock);
		}
		break;
	case ERcvStepType::PlaySound:
		if (Step.Sound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, Step.Sound, Target->GetActorLocation());
		}
		break;
	case ERcvStepType::ShowActors:
		Target->SetActorHiddenInGame(false);
		Target->SetActorEnableCollision(true);
		break;
	case ERcvStepType::HideActors:
		Target->SetActorHiddenInGame(true);
		Target->SetActorEnableCollision(false);
		break;
	}
}
