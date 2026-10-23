#include "Narrative/RcvNeonLight.h"

#include "Components/AudioComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ARcvNeonLight::ARcvNeonLight()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

	// Segnaposto per il blockout: plafoniera 120 x 20 x 6 cm.
	Fixture = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Fixture"));
	Fixture->SetupAttachment(GetRootComponent());
	Fixture->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Fixture->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded())
	{
		Fixture->SetStaticMesh(Cube.Object);
		Fixture->SetRelativeScale3D(FVector(1.2f, 0.2f, 0.06f));
	}

	Light = CreateDefaultSubobject<URectLightComponent>(TEXT("Light"));
	Light->SetupAttachment(GetRootComponent());
	Light->SetRelativeLocation(FVector(0.f, 0.f, -4.f));
	Light->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	Light->IntensityUnits = ELightUnits::Lumens;
	Light->Intensity = OnIntensityLumens;
	Light->bUseTemperature = true;
	Light->Temperature = 4300.f;
	Light->SourceWidth = 120.f;
	Light->SourceHeight = 12.f;
	Light->BarnDoorAngle = 70.f;
	Light->AttenuationRadius = 900.f;

	Hum = CreateDefaultSubobject<UAudioComponent>(TEXT("Hum"));
	Hum->SetupAttachment(GetRootComponent());
	Hum->bAutoActivate = false;
}

void ARcvNeonLight::BeginPlay()
{
	Super::BeginPlay();

	if (HumLoop)
	{
		Hum->SetSound(HumLoop);
	}
	SetState(InitialState, false);
}

void ARcvNeonLight::SetState(ERcvNeonState NewState, bool bPlaySound)
{
	const bool bWasLit = State != ERcvNeonState::Off;
	State = NewState;

	SetActorTickEnabled(State == ERcvNeonState::Flicker);
	FlickerTimer = 0.f;
	ApplyLit(State != ERcvNeonState::Off);

	if (bPlaySound)
	{
		USoundBase* Sound = nullptr;
		if (bWasLit && State == ERcvNeonState::Off)
		{
			Sound = OffSound;
		}
		else if (!bWasLit && State != ERcvNeonState::Off)
		{
			Sound = OnSound;
		}
		if (Sound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
		}
	}
}

void ARcvNeonLight::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FlickerTimer -= DeltaSeconds;
	if (FlickerTimer <= 0.f)
	{
		// Tubo che fatica ad accendersi: lunghe fasi accese interrotte da raffiche di spegnimenti brevi.
		bFlickerLit = !bFlickerLit;
		FlickerTimer = bFlickerLit ? FMath::FRandRange(0.05f, 2.5f) : FMath::FRandRange(0.03f, 0.18f);
		ApplyLit(bFlickerLit);
	}
}

void ARcvNeonLight::ApplyLit(bool bLit)
{
	Light->SetIntensity(bLit ? OnIntensityLumens : 0.f);
	Light->SetVisibility(bLit);
	if (bLit && Hum->Sound && !Hum->IsPlaying())
	{
		Hum->Play();
	}
	else if (!bLit && State == ERcvNeonState::Off)
	{
		Hum->Stop();
	}
}
