#include "Player/RcvCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Flashlight/RcvFlashlightComponent.h"
#include "Interaction/RcvInteractionComponent.h"
#include "Inventory/RcvInventoryComponent.h"
#include "Player/RcvPlayerController.h"
#include "Player/RcvStaminaComponent.h"
#include "Ricovero.h"

ARcvCharacter::ARcvCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
	BaseEyeHeight = 70.f;
	CrouchedEyeHeight = 40.f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetFieldOfView(85.f);

	Stamina = CreateDefaultSubobject<URcvStaminaComponent>(TEXT("Stamina"));

	BreathAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("BreathAudio"));
	BreathAudio->SetupAttachment(FirstPersonCamera);
	BreathAudio->bAutoActivate = false;
	BreathAudio->bAllowSpatialization = false;

	Inventory = CreateDefaultSubobject<URcvInventoryComponent>(TEXT("Inventory"));
	Interaction = CreateDefaultSubobject<URcvInteractionComponent>(TEXT("Interaction"));

	// Torcia tenuta poco sotto e a destra degli occhi, come in mano.
	Flashlight = CreateDefaultSubobject<URcvFlashlightComponent>(TEXT("Flashlight"));
	Flashlight->SetupAttachment(FirstPersonCamera);
	Flashlight->SetRelativeLocation(FVector(10.f, 18.f, -14.f));

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->MaxWalkSpeed = WalkSpeed;
	MoveComp->MaxWalkSpeedCrouched = CrouchSpeed;
	MoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	MoveComp->bCanWalkOffLedgesWhenCrouching = true;
	MoveComp->SetCrouchedHalfHeight(50.f);
	MoveComp->BrakingDecelerationWalking = 1400.f;
	MoveComp->GroundFriction = 6.f;
	MoveComp->MaxAcceleration = 1200.f;

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

void ARcvCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

	if (BreathLoop)
	{
		BreathAudio->SetSound(BreathLoop);
		BreathAudio->SetVolumeMultiplier(0.f);
		BreathAudio->Play();
	}
}

void ARcvCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (const APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (!RuntimeMappingContext)
			{
				RuntimeMappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Runtime"));
				BuildMappingContext(RuntimeMappingContext);
			}
			Subsystem->RemoveMappingContext(RuntimeMappingContext);
			Subsystem->AddMappingContext(RuntimeMappingContext, 0);
		}
	}
}

namespace
{
	/** IA_Move è un Axis2D con X = destra e Y = avanti: i tasti singoli danno X positivo, quindi si ruotano/negano. */
	void MapKey(UInputMappingContext* Context, const UInputAction* Action, const FKey& Key, bool bToForwardAxis = false, bool bNegate = false)
	{
		if (!Action)
		{
			return;
		}
		FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);
		if (bToForwardAxis)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(Context));
		}
		if (bNegate)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(Context));
		}
	}
}

void ARcvCharacter::BuildMappingContext(UInputMappingContext* Context) const
{
	MapKey(Context, MoveAction, EKeys::W, true, false);
	MapKey(Context, MoveAction, EKeys::S, true, true);
	MapKey(Context, MoveAction, EKeys::D);
	MapKey(Context, MoveAction, EKeys::A, false, true);
	MapKey(Context, MoveAction, EKeys::Gamepad_Left2D);

	MapKey(Context, LookAction, EKeys::Mouse2D);
	MapKey(Context, LookAction, EKeys::Gamepad_Right2D);

	MapKey(Context, SprintAction, EKeys::LeftShift);
	MapKey(Context, SprintAction, EKeys::Gamepad_LeftThumbstick);

	MapKey(Context, CrouchAction, EKeys::LeftControl);
	MapKey(Context, CrouchAction, EKeys::C);
	MapKey(Context, CrouchAction, EKeys::Gamepad_RightThumbstick);

	MapKey(Context, InteractAction, EKeys::E);
	MapKey(Context, InteractAction, EKeys::Gamepad_FaceButton_Bottom);

	MapKey(Context, FlashlightAction, EKeys::F);
	MapKey(Context, FlashlightAction, EKeys::Gamepad_FaceButton_Top);

	MapKey(Context, ReloadAction, EKeys::R);
	MapKey(Context, ReloadAction, EKeys::Gamepad_FaceButton_Left);
}

void ARcvCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		UE_LOG(LogRcv, Error, TEXT("ARcvCharacter richiede Enhanced Input (vedi Config/DefaultInput.ini)."));
		return;
	}

	if (MoveAction)
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARcvCharacter::Move);
		Input->BindAction(MoveAction, ETriggerEvent::Completed, this, &ARcvCharacter::Move);
	}
	if (LookAction)
	{
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARcvCharacter::Look);
	}
	if (SprintAction)
	{
		Input->BindAction(SprintAction, ETriggerEvent::Started, this, &ARcvCharacter::SprintStarted);
		Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &ARcvCharacter::SprintStopped);
	}
	if (CrouchAction)
	{
		Input->BindAction(CrouchAction, ETriggerEvent::Started, this, &ARcvCharacter::CrouchStarted);
		Input->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ARcvCharacter::CrouchStopped);
	}
	if (InteractAction)
	{
		Input->BindAction(InteractAction, ETriggerEvent::Started, this, &ARcvCharacter::InteractPressed);
	}
	if (FlashlightAction)
	{
		Input->BindAction(FlashlightAction, ETriggerEvent::Started, this, &ARcvCharacter::FlashlightPressed);
	}
	if (ReloadAction)
	{
		Input->BindAction(ReloadAction, ETriggerEvent::Started, this, &ARcvCharacter::ReloadPressed);
	}
}

void ARcvCharacter::Move(const FInputActionValue& Value)
{
	LastMoveInput = Value.Get<FVector2D>();
	if (!LastMoveInput.IsNearlyZero())
	{
		AddMovementInput(GetActorForwardVector(), LastMoveInput.Y);
		AddMovementInput(GetActorRightVector(), LastMoveInput.X);
	}
}

void ARcvCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	// Mouse e stick danno Y positivo verso l'alto; il pitch del controller è invertito.
	AddControllerPitchInput(-Axis.Y);
}

void ARcvCharacter::SprintStarted()
{
	bWantsToSprint = true;
}

void ARcvCharacter::SprintStopped()
{
	bWantsToSprint = false;
}

void ARcvCharacter::CrouchStarted()
{
	Crouch();
}

void ARcvCharacter::CrouchStopped()
{
	UnCrouch();
}

void ARcvCharacter::InteractPressed()
{
	// Mentre si legge un documento, lo stesso tasto lo chiude.
	if (ARcvPlayerController* PC = Cast<ARcvPlayerController>(Controller))
	{
		if (PC->IsReadingDocument())
		{
			PC->CloseDocument();
			return;
		}
	}
	Interaction->TryInteract();
}

void ARcvCharacter::FlashlightPressed()
{
	Flashlight->Toggle();
}

void ARcvCharacter::ReloadPressed()
{
	if (!Flashlight->HasFlashlight() || Flashlight->GetBatteryRatio() >= 0.99f)
	{
		return;
	}
	if (Inventory->ConsumeBattery())
	{
		Flashlight->AddCharge(Flashlight->ChargePerBattery);
	}
}

void ARcvCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	const float OldEyeHeight = BaseEyeHeight;
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	// La capsula si abbassa di colpo: compensiamo e poi riportiamo l'offset a zero gradualmente.
	CrouchEyeOffsetZ += OldEyeHeight - BaseEyeHeight + HalfHeightAdjust;
}

void ARcvCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	const float OldEyeHeight = BaseEyeHeight;
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CrouchEyeOffsetZ += OldEyeHeight - BaseEyeHeight - HalfHeightAdjust;
}

void ARcvCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateGait();
	UpdateCamera(DeltaSeconds);
	UpdateFootsteps(DeltaSeconds);
	UpdateBreath(DeltaSeconds);
}

void ARcvCharacter::UpdateGait()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	const bool bMoving = MoveComp->IsMovingOnGround() && GetVelocity().Size2D() > 10.f;
	// Si corre solo andando in avanti, in piedi e con stamina disponibile.
	const bool bCanSprintNow = bWantsToSprint && !bIsCrouched && LastMoveInput.Y > 0.5f && Stamina->CanSprint();

	if (bIsCrouched)
	{
		Gait = bMoving ? ERcvGait::Crouch : ERcvGait::Idle;
	}
	else if (bMoving)
	{
		Gait = bCanSprintNow ? ERcvGait::Sprint : ERcvGait::Walk;
	}
	else
	{
		Gait = ERcvGait::Idle;
	}

	MoveComp->MaxWalkSpeed = bCanSprintNow ? SprintSpeed : WalkSpeed;
	Stamina->SetDraining(Gait == ERcvGait::Sprint);
}

void ARcvCharacter::UpdateCamera(float DeltaSeconds)
{
	CrouchEyeOffsetZ = FMath::FInterpTo(CrouchEyeOffsetZ, 0.f, DeltaSeconds, CrouchCameraInterpSpeed);

	FVector TargetBob = FVector::ZeroVector;
	if (bHeadBobEnabled && Gait != ERcvGait::Idle)
	{
		float Multiplier = 1.f;
		float Stride = WalkStride;
		if (Gait == ERcvGait::Sprint)
		{
			Multiplier = SprintBobMultiplier;
			Stride = SprintStride;
		}
		else if (Gait == ERcvGait::Crouch)
		{
			Multiplier = CrouchBobMultiplier;
			Stride = CrouchStride;
		}

		// Un ciclo completo di bob ogni due passi, così il punto più basso coincide con l'appoggio del piede.
		const float Speed = static_cast<float>(GetVelocity().Size2D());
		HeadBobPhase += DeltaSeconds * Speed / (2.f * Stride) * UE_TWO_PI;
		HeadBobPhase = FMath::Fmod(HeadBobPhase, UE_TWO_PI);

		TargetBob.Z = FMath::Abs(FMath::Sin(HeadBobPhase)) * -HeadBobVerticalAmplitude * Multiplier;
		TargetBob.Y = FMath::Sin(HeadBobPhase) * HeadBobLateralAmplitude * Multiplier;
	}
	HeadBobOffset = FMath::VInterpTo(HeadBobOffset, TargetBob, DeltaSeconds, 12.f);

	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight + CrouchEyeOffsetZ) + HeadBobOffset);
}

void ARcvCharacter::UpdateFootsteps(float DeltaSeconds)
{
	if (Gait == ERcvGait::Idle)
	{
		// Al prossimo movimento il primo passo arriva subito, non dopo un passo intero.
		StrideDistance = WalkStride * 0.5f;
		return;
	}

	const float Stride = Gait == ERcvGait::Sprint ? SprintStride : (Gait == ERcvGait::Crouch ? CrouchStride : WalkStride);
	StrideDistance += static_cast<float>(GetVelocity().Size2D()) * DeltaSeconds;
	if (StrideDistance >= Stride)
	{
		StrideDistance -= Stride;
		PlayFootstep();
	}
}

void ARcvCharacter::PlayFootstep()
{
	const FVector Start = GetActorLocation();
	const FVector End = Start - FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 30.f);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RcvFootstep), false, this);
	Params.bReturnPhysicalMaterial = true;

	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return;
	}

	const EPhysicalSurface Surface = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
	USoundBase* Sound = DefaultFootstepSound;
	if (const TObjectPtr<USoundBase>* Found = FootstepSounds.Find(Surface))
	{
		Sound = *Found;
	}
	if (!Sound)
	{
		return;
	}

	float Volume = WalkFootstepVolume;
	if (Gait == ERcvGait::Sprint)
	{
		Volume = SprintFootstepVolume;
	}
	else if (Gait == ERcvGait::Crouch)
	{
		Volume = CrouchFootstepVolume;
	}

	UGameplayStatics::PlaySoundAtLocation(this, Sound, Hit.ImpactPoint, Volume, FMath::FRandRange(0.95f, 1.05f));
}

void ARcvCharacter::UpdateBreath(float DeltaSeconds)
{
	if (!BreathAudio->IsPlaying())
	{
		return;
	}

	const float Fatigue = 1.f - Stamina->GetStaminaRatio();
	// Il respiro si sente solo da metà stamina in giù e resta forte finché si è esausti.
	float Target = static_cast<float>(FMath::GetMappedRangeValueClamped(FVector2D(0.5, 1.0), BreathVolumeRange, static_cast<double>(Fatigue)));
	if (Stamina->IsExhausted())
	{
		Target = BreathVolumeRange.Y;
	}

	BreathVolume = FMath::FInterpTo(BreathVolume, Target, DeltaSeconds, BreathVolumeInterpSpeed);
	BreathAudio->SetVolumeMultiplier(FMath::Max(BreathVolume, 0.001f));
}
