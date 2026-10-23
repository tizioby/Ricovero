#include "Flashlight/RcvFlashlightComponent.h"

#include "Kismet/GameplayStatics.h"

URcvFlashlightComponent::URcvFlashlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Dopo la camera, così il ritardo di rotazione usa la posa di questo frame.
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	IntensityUnits = ELightUnits::Lumens;
	Intensity = FullIntensityLumens;
	InnerConeAngle = 18.f;
	OuterConeAngle = 32.f;
	AttenuationRadius = 2200.f;
	bUseTemperature = true;
	Temperature = 5200.f;
	CastShadows = true;
	bUseInverseSquaredFalloff = true;
	SourceRadius = 1.5f;
	SoftSourceRadius = 0.f;
	VolumetricScatteringIntensity = 1.5f;
	SetVisibility(false);
}

void URcvFlashlightComponent::BeginPlay()
{
	Super::BeginPlay();

	Battery = BatteryLife;
	// La rotazione la gestiamo noi in Tick per avere il ritardo.
	SetUsingAbsoluteRotation(true);
	if (const USceneComponent* Parent = GetAttachParent())
	{
		SetWorldRotation(Parent->GetComponentRotation());
	}
	SetVisibility(bIsOn);
}

void URcvFlashlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (const USceneComponent* Parent = GetAttachParent())
	{
		const FQuat Target = Parent->GetComponentQuat();
		const float Alpha = FMath::Clamp(DeltaTime * RotationLagSpeed, 0.f, 1.f);
		SetWorldRotation(FQuat::Slerp(GetComponentQuat(), Target, Alpha));
	}

	if (bIsOn)
	{
		Battery = FMath::Max(0.f, Battery - DeltaTime);
		if (Battery <= 0.f)
		{
			SetOn(false);
		}
	}

	UpdateIntensity(DeltaTime);

	const float Ratio = GetBatteryRatio();
	if (FMath::Abs(Ratio - LastBroadcastRatio) >= 0.01f)
	{
		LastBroadcastRatio = Ratio;
		OnBatteryChanged.Broadcast(Ratio);
	}
}

void URcvFlashlightComponent::UpdateIntensity(float DeltaTime)
{
	if (!bIsOn)
	{
		return;
	}

	const float Ratio = GetBatteryRatio();
	float Scale = 1.f;

	if (Ratio < LowBatteryThreshold)
	{
		// La luce cala con la carica e tremola a scatti, sempre più spesso man mano che si scarica.
		const float Low = Ratio / LowBatteryThreshold;
		Scale = FMath::Lerp(MinLowBatteryIntensity, 1.f, Low);

		FlickerTimer -= DeltaTime;
		if (FlickerTimer <= 0.f)
		{
			const bool bDropout = FMath::FRand() < FMath::Lerp(0.35f, 0.08f, Low);
			FlickerValue = bDropout ? FMath::FRandRange(0.05f, 0.4f) : FMath::FRandRange(0.85f, 1.f);
			FlickerTimer = bDropout ? FMath::FRandRange(0.04f, 0.12f) : FMath::FRandRange(0.2f, FMath::Lerp(0.6f, 2.5f, Low));
		}
		Scale *= FlickerValue;
	}

	SetIntensity(FullIntensityLumens * Scale);
}

void URcvFlashlightComponent::GiveFlashlight(bool bTurnOn)
{
	bHasFlashlight = true;
	if (bTurnOn)
	{
		SetOn(true);
	}
}

void URcvFlashlightComponent::Toggle()
{
	SetOn(!bIsOn);
}

void URcvFlashlightComponent::SetOn(bool bNewOn)
{
	if (!bHasFlashlight)
	{
		return;
	}
	if (bNewOn && Battery <= 0.f)
	{
		PlaySound(EmptyClickSound);
		return;
	}
	if (bNewOn == bIsOn)
	{
		return;
	}

	bIsOn = bNewOn;
	SetVisibility(bIsOn);
	FlickerTimer = 0.f;
	FlickerValue = 1.f;
	PlaySound(bIsOn ? ToggleOnSound : ToggleOffSound);
}

void URcvFlashlightComponent::AddCharge(float Ratio)
{
	SetBatteryRatio(GetBatteryRatio() + Ratio);
}

void URcvFlashlightComponent::SetBatteryRatio(float Ratio)
{
	Battery = FMath::Clamp(Ratio, 0.f, 1.f) * BatteryLife;
	if (Battery <= 0.f)
	{
		SetOn(false);
	}
}

void URcvFlashlightComponent::PlaySound(USoundBase* Sound) const
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}
