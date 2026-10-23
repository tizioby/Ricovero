#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RcvActStart.generated.h"

class UArrowComponent;

/** Punto di partenza di un atto: checkpoint, ripartenza dopo la morte e Rcv.GoToAct. */
UCLASS()
class RICOVERO_API ARcvActStart : public AActor
{
	GENERATED_BODY()

public:
	ARcvActStart();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atto", meta = (ClampMin = "1", ClampMax = "4"))
	int32 Act = 1;

	static ARcvActStart* Find(const UObject* WorldContext, int32 Act);

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
	TObjectPtr<UArrowComponent> Arrow;
#endif
};
