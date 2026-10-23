#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RcvActTrigger.generated.h"

class UBoxComponent;

/** Volume che segna l'ingresso in un nuovo atto (checkpoint) oppure la fine del gioco. */
UCLASS()
class RICOVERO_API ARcvActTrigger : public AActor
{
	GENERATED_BODY()

public:
	ARcvActTrigger();

	/** Atto in cui si entra attraversando il volume. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atto", meta = (ClampMin = "1", ClampMax = "4"))
	int32 Act = 2;

	/** Se vero, attraversarlo termina la partita e mostra la schermata finale. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atto")
	bool bEndsGame = false;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Componenti")
	TObjectPtr<UBoxComponent> Box;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bTriggered = false;
};
