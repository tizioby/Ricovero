#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/RcvInteractable.h"
#include "Inventory/RcvInventoryComponent.h"
#include "RcvPickup.generated.h"

class UStaticMeshComponent;
class USoundBase;

UENUM(BlueprintType)
enum class ERcvPickupType : uint8
{
	Flashlight,
	Battery,
	Key,
	Document
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRcvPickedUp, ARcvCharacter*, Character);

/**
 * Oggetto raccoglibile. Ogni istanza nel livello si configura dal pannello Details:
 * tipo, id della chiave, testo del documento.
 */
UCLASS()
class RICOVERO_API ARcvPickup : public AActor, public IRcvInteractable
{
	GENERATED_BODY()

public:
	ARcvPickup();

	virtual bool CanInteract_Implementation(ARcvCharacter* Interactor) const override;
	virtual void Interact_Implementation(ARcvCharacter* Interactor) override;
	virtual void SetFocused_Implementation(bool bFocused) override;

	/** Usato dagli eventi scriptati (es. il primo documento fa partire un suono). */
	UPROPERTY(BlueprintAssignable, Category = "Raccolta")
	FRcvPickedUp OnPickedUp;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componenti")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raccolta")
	ERcvPickupType Type = ERcvPickupType::Battery;

	/** Solo per le chiavi: deve corrispondere a RequiredKey della porta. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raccolta", meta = (EditCondition = "Type == ERcvPickupType::Key", EditConditionHides))
	FName KeyId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raccolta", meta = (EditCondition = "Type == ERcvPickupType::Document", EditConditionHides))
	FRcvDocument Document;

	UPROPERTY(EditAnywhere, Category = "Raccolta")
	TObjectPtr<USoundBase> PickupSound;

private:
	bool bCollected = false;
};
