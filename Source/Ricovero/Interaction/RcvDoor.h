#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/RcvInteractable.h"
#include "RcvDoor.generated.h"

class UStaticMeshComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRcvDoorEvent, ARcvDoor*, Door);

/**
 * Porta a battente. Si apre allontanandosi dal giocatore, può essere chiusa a chiave
 * e può essere sbattuta dagli eventi scriptati (Slam).
 * L'origine dell'attore è il cardine: la mesh dell'anta va spostata in modo che il bordo stia sull'origine.
 */
UCLASS()
class RICOVERO_API ARcvDoor : public AActor, public IRcvInteractable
{
	GENERATED_BODY()

public:
	ARcvDoor();

	virtual void Tick(float DeltaSeconds) override;

	virtual bool CanInteract_Implementation(ARcvCharacter* Interactor) const override;
	virtual void Interact_Implementation(ARcvCharacter* Interactor) override;
	virtual void SetFocused_Implementation(bool bFocused) override;

	/** Apre verso il lato opposto a FromLocation. */
	UFUNCTION(BlueprintCallable, Category = "Porta")
	void Open(FVector FromLocation);

	UFUNCTION(BlueprintCallable, Category = "Porta")
	void Close();

	/** Chiusura violenta per gli eventi: veloce, con suono forte. Opzionalmente la blocca. */
	UFUNCTION(BlueprintCallable, Category = "Porta")
	void Slam(bool bLockAfter = false);

	UFUNCTION(BlueprintCallable, Category = "Porta")
	void SetLocked(bool bNewLocked) { bLocked = bNewLocked; }

	UFUNCTION(BlueprintPure, Category = "Porta")
	bool IsOpen() const { return !FMath::IsNearlyZero(TargetYaw); }

	UPROPERTY(BlueprintAssignable, Category = "Porta")
	FRcvDoorEvent OnOpened;

	/** Il giocatore ha provato ad aprirla ma è chiusa a chiave. */
	UPROPERTY(BlueprintAssignable, Category = "Porta")
	FRcvDoorEvent OnLockedAttempt;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componenti")
	TObjectPtr<USceneComponent> Frame;

	/** Cardine: ruota attorno a Z. L'anta è figlia del cardine. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componenti")
	TObjectPtr<USceneComponent> Hinge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componenti")
	TObjectPtr<UStaticMeshComponent> Leaf;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Porta")
	bool bLocked = false;

	/** Chiave necessaria per sbloccarla. Vuoto = si sblocca solo da evento. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Porta")
	FName RequiredKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Porta")
	bool bStartsOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Porta")
	float OpenAngle = 95.f;

	/** Gradi al secondo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Porta")
	float SwingSpeed = 110.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Porta")
	float SlamSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Porta|Suoni")
	TObjectPtr<USoundBase> OpenSound;

	UPROPERTY(EditAnywhere, Category = "Porta|Suoni")
	TObjectPtr<USoundBase> CloseSound;

	UPROPERTY(EditAnywhere, Category = "Porta|Suoni")
	TObjectPtr<USoundBase> LockedSound;

	UPROPERTY(EditAnywhere, Category = "Porta|Suoni")
	TObjectPtr<USoundBase> UnlockSound;

	UPROPERTY(EditAnywhere, Category = "Porta|Suoni")
	TObjectPtr<USoundBase> SlamSound;

	virtual void BeginPlay() override;

private:
	void PlayDoorSound(USoundBase* Sound) const;

	float CurrentYaw = 0.f;
	float TargetYaw = 0.f;
	float CurrentSpeed = 0.f;
};
