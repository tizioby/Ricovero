#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Chaos/ChaosEngineInterface.h"
#include "RcvCharacter.generated.h"

class UCameraComponent;
class UAudioComponent;
class UInputAction;
class UInputMappingContext;
class USoundBase;
class URcvStaminaComponent;
struct FInputActionValue;

/** Andatura corrente, usata per velocità, passi e head bob. */
UENUM(BlueprintType)
enum class ERcvGait : uint8
{
	Idle,
	Walk,
	Sprint,
	Crouch
};

/**
 * Protagonista in prima persona: cammina, corre con stamina limitata, si accuccia.
 * Head bob procedurale, passi per superficie e respiro legato alla stamina.
 * Gli asset (input, suoni) si assegnano nel Blueprint figlio BP_RcvCharacter.
 */
UCLASS()
class RICOVERO_API ARcvCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ARcvCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UFUNCTION(BlueprintPure, Category = "Movimento")
	ERcvGait GetGait() const { return Gait; }

	UFUNCTION(BlueprintPure, Category = "Movimento")
	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componenti")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componenti")
	TObjectPtr<URcvStaminaComponent> Stamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Componenti")
	TObjectPtr<UAudioComponent> BreathAudio;

	// --- Input ---

	/**
	 * Tasti costruiti in C++ a runtime a partire dalle Input Action qui sotto,
	 * così le mappature (e i modificatori di WASD) non dipendono da un asset fatto a mano.
	 */
	virtual void BuildMappingContext(UInputMappingContext* Context) const;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> RuntimeMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;

	// --- Movimento (cm/s) ---

	UPROPERTY(EditDefaultsOnly, Category = "Movimento")
	float WalkSpeed = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movimento")
	float SprintSpeed = 350.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movimento")
	float CrouchSpeed = 90.f;

	/** Velocità con cui la camera raggiunge la nuova altezza quando ci si accuccia. */
	UPROPERTY(EditDefaultsOnly, Category = "Movimento")
	float CrouchCameraInterpSpeed = 10.f;

	// --- Head bob ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Head Bob")
	bool bHeadBobEnabled = true;

	/** Ampiezza verticale (cm) camminando; corsa e accucciata la scalano. */
	UPROPERTY(EditDefaultsOnly, Category = "Head Bob")
	float HeadBobVerticalAmplitude = 0.9f;

	/** Ampiezza laterale (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "Head Bob")
	float HeadBobLateralAmplitude = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Head Bob")
	float SprintBobMultiplier = 1.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Head Bob")
	float CrouchBobMultiplier = 0.6f;

	// --- Passi ---

	/** Distanza percorsa tra un passo e l'altro (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "Passi")
	float WalkStride = 75.f;

	UPROPERTY(EditDefaultsOnly, Category = "Passi")
	float SprintStride = 110.f;

	UPROPERTY(EditDefaultsOnly, Category = "Passi")
	float CrouchStride = 55.f;

	/** Suono del passo per tipo di superficie (Physical Material). */
	UPROPERTY(EditDefaultsOnly, Category = "Passi")
	TMap<TEnumAsByte<EPhysicalSurface>, TObjectPtr<USoundBase>> FootstepSounds;

	/** Usato quando la superficie non ha un suono dedicato. */
	UPROPERTY(EditDefaultsOnly, Category = "Passi")
	TObjectPtr<USoundBase> DefaultFootstepSound;

	UPROPERTY(EditDefaultsOnly, Category = "Passi")
	float WalkFootstepVolume = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "Passi")
	float SprintFootstepVolume = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Passi")
	float CrouchFootstepVolume = 0.25f;

	// --- Respiro ---

	/** Loop di respiro affannato: il volume sale quando la stamina scende. */
	UPROPERTY(EditDefaultsOnly, Category = "Respiro")
	TObjectPtr<USoundBase> BreathLoop;

	/** Volume del respiro a stamina piena e a stamina vuota. */
	UPROPERTY(EditDefaultsOnly, Category = "Respiro")
	FVector2D BreathVolumeRange = FVector2D(0.f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Respiro")
	float BreathVolumeInterpSpeed = 2.f;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SprintStarted();
	void SprintStopped();
	void CrouchStarted();
	void CrouchStopped();

	void UpdateGait();
	void UpdateCamera(float DeltaSeconds);
	void UpdateFootsteps(float DeltaSeconds);
	void UpdateBreath(float DeltaSeconds);
	void PlayFootstep();

	ERcvGait Gait = ERcvGait::Idle;
	bool bWantsToSprint = false;
	FVector2D LastMoveInput = FVector2D::ZeroVector;

	/** Offset che rende graduale il cambio di altezza della camera quando ci si accuccia. */
	float CrouchEyeOffsetZ = 0.f;
	float HeadBobPhase = 0.f;
	FVector HeadBobOffset = FVector::ZeroVector;
	float StrideDistance = 0.f;
	float BreathVolume = 0.f;
};
