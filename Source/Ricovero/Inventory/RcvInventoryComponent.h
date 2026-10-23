#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RcvInventoryComponent.generated.h"

/** Un documento raccolto: si può rileggere in qualsiasi momento. */
USTRUCT(BlueprintType)
struct FRcvDocument
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Documento")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Documento")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Documento", meta = (MultiLine = true))
	FText Body;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRcvInventoryChanged);

/**
 * Inventario minimale: torcia, chiavi, pile, documenti.
 * Ogni oggetto ha uno scopo; niente peso, slot o crafting.
 */
UCLASS(ClassGroup = (Ricovero), meta = (BlueprintSpawnableComponent))
class RICOVERO_API URcvInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	void AddKey(FName KeyId);

	UFUNCTION(BlueprintPure, Category = "Inventario")
	bool HasKey(FName KeyId) const { return Keys.Contains(KeyId); }

	UFUNCTION(BlueprintCallable, Category = "Inventario")
	void AddBatteries(int32 Count = 1);

	/** Consuma una pila se disponibile. */
	UFUNCTION(BlueprintCallable, Category = "Inventario")
	bool ConsumeBattery();

	UFUNCTION(BlueprintPure, Category = "Inventario")
	int32 GetBatteryCount() const { return Batteries; }

	UFUNCTION(BlueprintCallable, Category = "Inventario")
	void AddDocument(const FRcvDocument& Document);

	UFUNCTION(BlueprintPure, Category = "Inventario")
	const TArray<FRcvDocument>& GetDocuments() const { return Documents; }

	UPROPERTY(BlueprintAssignable, Category = "Inventario")
	FRcvInventoryChanged OnInventoryChanged;

	/** Numero massimo di pile trasportabili: evita di accumularle e mantiene la tensione. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventario")
	int32 MaxBatteries = 3;

	/** Stato completo, usato dal salvataggio. */
	TSet<FName> Keys;
	int32 Batteries = 0;
	TArray<FRcvDocument> Documents;
};
