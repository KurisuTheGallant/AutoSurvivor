// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoSurvivorCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Bullet.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyCharacter.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AAutoSurvivorCharacter

AAutoSurvivorCharacter::AAutoSurvivorCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	CameraBoom->TargetArmLength = 1200.0f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bUsePawnControlRotation = false;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AAutoSurvivorCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		PlayerController->ConsoleCommand("viewmode unlit");
	}

	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AAutoSurvivorCharacter::FireWeapon, FireRate, true);
}

//////////////////////////////////////////////////////////////////////////
// WEAPON & LEVELING LOGIC

AActor* AAutoSurvivorCharacter::GetNearestEnemy()
{
	TArray<AActor*> AllEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyCharacter::StaticClass(), AllEnemies);

	AActor* NearestEnemy = nullptr;
	float NearestDistanceSq = FLT_MAX;
	FVector MyLoc = GetActorLocation();

	for (AActor* Enemy : AllEnemies)
	{
		if (Enemy)
		{
			float DistSq = FVector::DistSquared(MyLoc, Enemy->GetActorLocation());
			if (DistSq < NearestDistanceSq)
			{
				NearestDistanceSq = DistSq;
				NearestEnemy = Enemy;
			}
		}
	}
	return NearestEnemy;
}

void AAutoSurvivorCharacter::FireWeapon()
{
	if (!BulletClass) return;

	AActor* Target = GetNearestEnemy();
	FRotator SpawnRotation = GetActorRotation();

	if (Target)
	{
		FVector Direction = Target->GetActorLocation() - GetActorLocation();
		SpawnRotation = Direction.Rotation();
	}

	FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 50.0f);

	GetWorld()->SpawnActor<ABullet>(BulletClass, SpawnLocation, SpawnRotation);
}

void AAutoSurvivorCharacter::AddExperience(float Amount)
{
	CurrentExperience += Amount;

	// Did we level up?
	if (CurrentExperience >= MaxExperience)
	{
		CurrentLevel++;
		CurrentExperience -= MaxExperience; // Keep overflow XP
		MaxExperience *= 1.2f; // Next level is 20% harder to reach

		// Debug Message on screen (Cyan color)
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("LEVEL UP! New Level: %d"), CurrentLevel));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AAutoSurvivorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAutoSurvivorCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAutoSurvivorCharacter::Look);
	}
}

void AAutoSurvivorCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AAutoSurvivorCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}	