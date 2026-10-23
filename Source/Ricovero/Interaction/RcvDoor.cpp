#include "Interaction/RcvDoor.h"

#include "Components/StaticMeshComponent.h"
#include "Inventory/RcvInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/RcvCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARcvDoor::ARcvDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Frame = CreateDefaultSubobject<USceneComponent>(TEXT("Frame"));
	SetRootComponent(Frame);

	Hinge = CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
	Hinge->SetupAttachment(Frame);

	Leaf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Leaf"));
	Leaf->SetupAttachment(Hinge);
	Leaf->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);

	// Segnaposto per il blockout: anta 100 x 5 x 210 cm con il bordo sul cardine.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded())
	{
		Leaf->SetStaticMesh(Cube.Object);
		Leaf->SetRelativeScale3D(FVector(1.f, 0.05f, 2.1f));
		Leaf->SetRelativeLocation(FVector(50.f, 0.f, 105.f));
	}
}

void ARcvDoor::BeginPlay()
{
	Super::BeginPlay();

	if (bStartsOpen)
	{
		CurrentYaw = TargetYaw = OpenAngle;
		Hinge->SetRelativeRotation(FRotator(0.f, CurrentYaw, 0.f));
	}
}

void ARcvDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CurrentYaw = FMath::FInterpConstantTo(CurrentYaw, TargetYaw, DeltaSeconds, CurrentSpeed);
	Hinge->SetRelativeRotation(FRotator(0.f, CurrentYaw, 0.f));

	if (FMath::IsNearlyEqual(CurrentYaw, TargetYaw, 0.1f))
	{
		SetActorTickEnabled(false);
	}
}

bool ARcvDoor::CanInteract_Implementation(ARcvCharacter* Interactor) const
{
	return true;
}

void ARcvDoor::Interact_Implementation(ARcvCharacter* Interactor)
{
	if (bLocked)
	{
		if (!RequiredKey.IsNone() && Interactor && Interactor->GetInventory()->HasKey(RequiredKey))
		{
			bLocked = false;
			PlayDoorSound(UnlockSound);
		}
		else
		{
			PlayDoorSound(LockedSound);
			OnLockedAttempt.Broadcast(this);
			return;
		}
	}

	if (IsOpen())
	{
		Close();
	}
	else if (Interactor)
	{
		Open(Interactor->GetActorLocation());
	}
}

void ARcvDoor::Open(FVector FromLocation)
{
	// Si apre dal lato opposto a chi spinge.
	const FVector Local = GetActorTransform().InverseTransformPosition(FromLocation);
	TargetYaw = Local.Y > 0.f ? -OpenAngle : OpenAngle;
	CurrentSpeed = SwingSpeed;
	SetActorTickEnabled(true);
	PlayDoorSound(OpenSound);
	OnOpened.Broadcast(this);
}

void ARcvDoor::Close()
{
	TargetYaw = 0.f;
	CurrentSpeed = SwingSpeed;
	SetActorTickEnabled(true);
	PlayDoorSound(CloseSound);
}

void ARcvDoor::Slam(bool bLockAfter)
{
	TargetYaw = 0.f;
	CurrentSpeed = SlamSpeed;
	SetActorTickEnabled(true);
	PlayDoorSound(SlamSound);
	if (bLockAfter)
	{
		bLocked = true;
	}
}

void ARcvDoor::SetFocused_Implementation(bool bFocused)
{
}

void ARcvDoor::PlayDoorSound(USoundBase* Sound) const
{
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, Leaf->GetComponentLocation());
	}
}
