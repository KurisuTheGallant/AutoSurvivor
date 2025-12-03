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
	CurrentHealth = MaxHealth;

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
	if (!BulletClass || bIsDead) return;

	AActor* Target = GetNearestEnemy();
	FRotator BaseRotation = GetActorRotation();

	if (Target)
	{
		FVector Direction = Target->GetActorLocation() - GetActorLocation();
		BaseRotation = Direction.Rotation();
	}

	FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 50.0f);

	// --- WEAPON LOGIC SWITCH ---
	switch (CurrentWeapon)
	{
	case EWeaponType::Pistol:
		// Fire 1 shot straight
		GetWorld()->SpawnActor<ABullet>(BulletClass, SpawnLocation, BaseRotation);
		break;

	case EWeaponType::Shotgun:
		// Fire 5 shots in a cone (-30 to +30 degrees)
		for (int i = -2; i <= 2; i++)
		{
			FRotator BulletRot = BaseRotation;
			BulletRot.Yaw += i * 10.0f; // Spread by 10 degrees each
			GetWorld()->SpawnActor<ABullet>(BulletClass, SpawnLocation, BulletRot);
		}
		break;

	case EWeaponType::MachineGun:
		// Fire 1 shot but with tiny random spread for "recoil" feel
		float RandomSpread = FMath::RandRange(-5.0f, 5.0f);
		FRotator BulletRot = BaseRotation;
		BulletRot.Yaw += RandomSpread;
		GetWorld()->SpawnActor<ABullet>(BulletClass, SpawnLocation, BulletRot);
		break;
	}

	PlayShootEffects();
}

void AAutoSurvivorCharacter::SetWeapon(EWeaponType NewWeapon)
{
	CurrentWeapon = NewWeapon;

	// Update Fire Rate based on weapon type
	switch (NewWeapon)
	{
	case EWeaponType::Pistol:
		FireRate = 0.5f;
		break;
	case EWeaponType::Shotgun:
		FireRate = 0.8f; // Shotgun is slower
		break;
	case EWeaponType::MachineGun:
		FireRate = 0.1f; // Machine Gun is extremely fast
		break;
	}

	// Reset the Timer so the new fire rate takes effect immediately
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AAutoSurvivorCharacter::FireWeapon, FireRate, true);
}

void AAutoSurvivorCharacter::AddExperience(float Amount)
{
	if (bIsDead) return;
	CurrentExperience += Amount;
	if (CurrentExperience >= MaxExperience)
	{
		CurrentLevel++;
		CurrentExperience -= MaxExperience;
		MaxExperience *= 1.2f;
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("LEVEL UP! New Level: %d"), CurrentLevel));
	}
}

void AAutoSurvivorCharacter::DamagePlayer(float Amount)
{
	if (bIsDead) return;
	CurrentHealth -= Amount;
	if (CurrentHealth <= 0.0f)
	{
		CurrentHealth = 0.0f;
		bIsDead = true;
		GetWorldTimerManager().ClearTimer(FireTimerHandle);
		OnDeath();
	}
}

void AAutoSurvivorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAutoSurvivorCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAutoSurvivorCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
}

void AAutoSurvivorCharacter::Move(const FInputActionValue& Value)
{
	if (this->bIsDead) return;
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (this->Controller != nullptr)
	{
		const FRotator Rotation = this->Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		this->AddMovementInput(ForwardDirection, MovementVector.Y);
		this->AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AAutoSurvivorCharacter::Look(const FInputActionValue& Value)
{
	if (this->bIsDead) return;
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (this->Controller != nullptr)
	{
		this->AddControllerYawInput(LookAxisVector.X);
		this->AddControllerPitchInput(LookAxisVector.Y);
	}
}