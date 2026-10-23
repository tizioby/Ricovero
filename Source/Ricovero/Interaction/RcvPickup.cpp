#include "Interaction/RcvPickup.h"

#include "Components/StaticMeshComponent.h"
#include "Flashlight/RcvFlashlightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/RcvCharacter.h"
#include "Player/RcvPlayerController.h"
#include "UObject/ConstructorHelpers.h"

ARcvPickup::ARcvPickup()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// Segnaposto per il blockout: un cubetto da 10 cm, finché non c'è la mesh vera.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (Cube.Succeeded())
	{
		Mesh->SetStaticMesh(Cube.Object);
		Mesh->SetRelativeScale3D(FVector(0.1f));
	}
}

bool ARcvPickup::CanInteract_Implementation(ARcvCharacter* Interactor) const
{
	return !bCollected;
}

void ARcvPickup::Interact_Implementation(ARcvCharacter* Interactor)
{
	if (bCollected || !Interactor)
	{
		return;
	}

	URcvInventoryComponent* Inventory = Interactor->GetInventory();
	switch (Type)
	{
	case ERcvPickupType::Flashlight:
		Interactor->GetFlashlight()->GiveFlashlight(true);
		break;
	case ERcvPickupType::Battery:
		if (Inventory->GetBatteryCount() >= Inventory->MaxBatteries)
		{
			// Tasche piene: la pila resta dov'è, non si spreca.
			return;
		}
		Inventory->AddBatteries(1);
		break;
	case ERcvPickupType::Key:
		Inventory->AddKey(KeyId);
		break;
	case ERcvPickupType::Document:
		Inventory->AddDocument(Document);
		if (ARcvPlayerController* PC = Cast<ARcvPlayerController>(Interactor->GetController()))
		{
			PC->ShowDocument(Document);
		}
		break;
	}

	bCollected = true;
	if (PickupSound)
	{
		UGameplayStatics::PlaySound2D(this, PickupSound);
	}
	OnPickedUp.Broadcast(Interactor);
	Destroy();
}

void ARcvPickup::SetFocused_Implementation(bool bFocused)
{
	// Contorno leggero via custom depth: il post-process del Passo 5 lo userà per un'evidenziazione discreta.
	Mesh->SetRenderCustomDepth(bFocused);
}
