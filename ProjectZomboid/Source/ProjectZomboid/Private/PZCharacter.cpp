// Fill out your copyright notice in the Description page of Project Settings.

#include "PZCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "PZStatComponent.h"
#include "Components/WidgetComponent.h"

APZCharacter::APZCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // We want to look at mouse, not movement dir
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// Create Stat Component
	StatComponent = CreateDefaultSubobject<UPZStatComponent>(TEXT("StatComponent"));

	// Create Stamina Widget Component
	StaminaWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("StaminaWidget"));
	StaminaWidget->SetupAttachment(RootComponent);
	StaminaWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // Above head
	StaminaWidget->SetWidgetSpace(EWidgetSpace::Screen);
	StaminaWidget->SetDrawAtDesiredSize(true);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.0f;
	CameraBoom->SetRelativeRotation(FRotator(-45.0f, -45.0f, 0.0f)); // Isometric view
	CameraBoom->bDoCollisionTest = false; // Don't want camera jumping on wall collision usually in PZ style
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void APZCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		
		// Show mouse cursor for PZ style interaction/aiming
		PlayerController->bShowMouseCursor = true;
	}
}

void APZCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LookAtMouseCursor();
}

void APZCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APZCharacter::Move);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APZCharacter::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APZCharacter::StopSprint);
	}
}

void APZCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// w를 눌렀을 때 X,Y가 모두 증가하는 방향으로 증가
		const FVector ForwardDirection = FVector(1.0f, 1.0f, 0.0f).GetSafeNormal();

		// d를 눌렀을 때 전진 방향과 수직인 방향( X 증가, Y 감소)
		const FVector RightDirection = FVector(1.0f, 1.0f, 0.0f).GetSafeNormal();

		//입력값 적용
		
		AddMovementInput(ForwardDirection, MovementVector.Y);	// W/S입력
		AddMovementInput(RightDirection, MovementVector.X);		// A/D 입력

		// 스테미나 로직
		if (bIsSprinting && (StatComponent->CurrentStamina <= 0.0f || GetVelocity().IsNearlyZero()))
		{
			StopSprint();
		}

		// 달리기 중지 로직
		if (bIsSprinting && !GetVelocity().IsNearlyZero())
		{
			StatComponent->ReduceStamina(SprintStaminaCost * GetWorld()->GetDeltaSeconds());
		}
	}
}

void APZCharacter::StartSprint()
{
	if (StatComponent && StatComponent->CurrentStamina > 0.0f)
	{
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void APZCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APZCharacter::LookAtMouseCursor()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FHitResult HitResult;
		if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
		{
			FVector TargetLocation = HitResult.ImpactPoint;
			FVector ActorLocation = GetActorLocation();
			TargetLocation.Z = ActorLocation.Z; // Keep rotation on the Z axis only

			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(ActorLocation, TargetLocation);
			
			// Smoothly interpolate rotation for better feel
			FRotator CurrentRotation = GetActorRotation();
			FRotator NewRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, GetWorld()->GetDeltaSeconds(), 10.0f);
			
			SetActorRotation(NewRotation);
		}
	}
}
