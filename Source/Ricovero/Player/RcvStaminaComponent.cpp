#include "Player/RcvStaminaComponent.h"

URcvStaminaComponent::URcvStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URcvStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
	Stamina = MaxStamina;
}

void URcvStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDraining)
	{
		TimeSinceDrain = 0.f;
		Stamina = FMath::Max(0.f, Stamina - DeltaTime);
		if (Stamina <= 0.f)
		{
			SetExhausted(true);
		}
		return;
	}

	TimeSinceDrain += DeltaTime;
	if (TimeSinceDrain >= RegenDelay && Stamina < MaxStamina)
	{
		Stamina = FMath::Min(MaxStamina, Stamina + DeltaTime * (MaxStamina / FullRegenTime));
	}

	if (bExhausted && GetStaminaRatio() >= RecoverThreshold)
	{
		SetExhausted(false);
	}
}

bool URcvStaminaComponent::CanSprint() const
{
	return !bExhausted && Stamina > 0.f;
}

void URcvStaminaComponent::SetExhausted(bool bNew)
{
	if (bExhausted != bNew)
	{
		bExhausted = bNew;
		OnExhaustedChanged.Broadcast(bExhausted);
	}
}
