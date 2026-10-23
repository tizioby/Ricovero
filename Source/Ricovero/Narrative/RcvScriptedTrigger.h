#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RcvScriptedTrigger.generated.h"

class UBoxComponent;
class USoundBase;

UENUM(BlueprintType)
enum class ERcvStepType : uint8
{
	/** Spegne i bersagli (neon o qualsiasi luce). */
	LightsOff,
	LightsOn,
	LightsFlicker,
	/** Sbatte le porte bersaglio; con bLock restano chiuse a chiave. */
	DoorSlam,
	/** Suono 3D sul primo bersaglio, o sul trigger se non ce ne sono. */
	PlaySound,
	/** Rende visibili i bersagli (es. la sagoma del mostro). */
	ShowActors,
	HideActors
};

/** Un passo di una sequenza scriptata. I bersagli vengono eseguiti uno dopo l'altro a distanza di Stagger. */
USTRUCT(BlueprintType)
struct FRcvScriptStep
{
	GENERATED_BODY()

	/** Attesa dopo la fine del passo precedente (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passo")
	float Delay = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passo")
	ERcvStepType Type = ERcvStepType::LightsOff;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passo")
	TArray<TObjectPtr<AActor>> Targets;

	/** Intervallo tra un bersaglio e il successivo (s): luci che si spengono in fila. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passo")
	float Stagger = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passo")
	TObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Passo")
	bool bLock = false;
};

/**
 * Volume che fa partire una sequenza di eventi quando il giocatore lo attraversa:
 * luci che saltano, rumori, porte che sbattono, apparizioni.
 */
UCLASS()
class RICOVERO_API ARcvScriptedTrigger : public AActor
{
	GENERATED_BODY()

public:
	ARcvScriptedTrigger();

	UFUNCTION(BlueprintCallable, Category = "Evento")
	void Run();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Componenti")
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Evento")
	TArray<FRcvScriptStep> Steps;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Evento")
	bool bOnlyOnce = true;

	/** Bersagli di ShowActors nascosti all'avvio. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Evento")
	bool bHideShowTargetsAtStart = true;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void ExecuteOnTarget(const FRcvScriptStep& Step, AActor* Target);

	bool bHasRun = false;
};
