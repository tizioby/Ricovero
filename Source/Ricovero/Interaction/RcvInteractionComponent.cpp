#include "Interaction/RcvInteractionComponent.h"

#include "Interaction/RcvInteractable.h"
#include "Player/RcvCharacter.h"

URcvInteractionComponent::URcvInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URcvInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ARcvCharacter* Character = Cast<ARcvCharacter>(GetOwner());
	if (!Character || !Character->IsPlayerControlled())
	{
		SetFocus(nullptr);
		return;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	Character->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	const FVector End = EyeLocation + EyeRotation.Vector() * InteractDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RcvInteract), false, Character);
	FHitResult Hit;
	AActor* Candidate = nullptr;
	if (GetWorld()->SweepSingleByChannel(Hit, EyeLocation, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(TraceRadius), Params))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<URcvInteractable>() && IRcvInteractable::Execute_CanInteract(HitActor, Character))
		{
			Candidate = HitActor;
		}
	}
	SetFocus(Candidate);
}

bool URcvInteractionComponent::TryInteract()
{
	AActor* Target = FocusedActor.Get();
	ARcvCharacter* Character = Cast<ARcvCharacter>(GetOwner());
	if (!Target || !Character)
	{
		return false;
	}
	IRcvInteractable::Execute_Interact(Target, Character);
	return true;
}

void URcvInteractionComponent::SetFocus(AActor* NewFocus)
{
	AActor* Old = FocusedActor.Get();
	if (Old == NewFocus)
	{
		return;
	}
	if (Old)
	{
		IRcvInteractable::Execute_SetFocused(Old, false);
	}
	FocusedActor = NewFocus;
	if (NewFocus)
	{
		IRcvInteractable::Execute_SetFocused(NewFocus, true);
	}
	OnFocusChanged.Broadcast(NewFocus);
}
