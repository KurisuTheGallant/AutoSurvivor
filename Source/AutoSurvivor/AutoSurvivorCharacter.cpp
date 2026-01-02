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

	// --- 1. CALCULATE DAMAGE BASED ON STATS ---
	// Base Damage (50) * Multiplier (e.g. 1.2 for +20% Might)
	float FinalDamage = 50.0f * DamageMultiplier;

	switch (CurrentWeapon)
	{
	case EWeaponType::Pistol:
	{
		ABullet* NewBullet = GetWorld()->SpawnActor<ABullet>(BulletClass, SpawnLocation, BaseRotation);

		// --- 2. INJECT DAMAGE INTO BULLET ---
		if (NewBullet)
		{
			NewBullet->Damage = FinalDamage;
		}
		break;
	}
	case EWeaponType::Shotgun:
		for (int i = -2; i <= 2; i++)
		{
			FRotator BulletRot = BaseRotation;
			BulletRot.Yaw += i * 10.0f;

			ABullet* NewBullet = GetWorld()->SpawnActor<ABullet>(BulletClass, SpawnLocation, BulletRot);
			if (NewBullet)
			{
				// Shotgun pellets often do slightly less damage per pellet since there are 5 of them
				// But for now, let's keep it full damage to feel powerful!
				NewBullet->Damage = FinalDamage;
			}
		}
		break;

	case EWeaponType::MachineGun:
	{
		float RandomSpread = FMath::RandRange(-5.0f, 5.0f);
		FRotator BulletRot = BaseRotation;
		BulletRot.Yaw += RandomSpread;

		ABullet* NewBullet = GetWorld()->SpawnActor<ABullet>(BulletClass, SpawnLocation, BulletRot);
		if (NewBullet)
		{
			// Machine guns fire fast, so we reduce damage per bullet to balance it
			NewBullet->Damage = FinalDamage * 0.5f;
		}
		break;
	}
	}

	PlayShootEffects();
}

void AAutoSurvivorCharacter::SetWeapon(EWeaponType NewWeapon)
{
	CurrentWeapon = NewWeapon;
	switch (NewWeapon)
	{
	case EWeaponType::Pistol: FireRate = 0.5f; break;
	case EWeaponType::Shotgun: FireRate = 0.8f; break;
	case EWeaponType::MachineGun: FireRate = 0.1f; break;
	}
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AAutoSurvivorCharacter::FireWeapon, FireRate, true);
}

// --- NEW: Returns a random list of upgrades ---
TArray<EUpgradeType> AAutoSurvivorCharacter::GetRandomUpgrades(int32 Count)
{
	TArray<EUpgradeType> AllOptions;

	// Add every possible upgrade to the pool
	AllOptions.Add(EUpgradeType::Weapon_Shotgun);
	AllOptions.Add(EUpgradeType::Weapon_MachineGun);
	AllOptions.Add(EUpgradeType::Stat_Health);
	AllOptions.Add(EUpgradeType::Stat_Speed);
	AllOptions.Add(EUpgradeType::Stat_Damage);

	// Shuffle the array (Fisher-Yates shuffle algorithm)
	int32 LastIndex = AllOptions.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 Index = FMath::RandRange(i, LastIndex);
		if (i != Index)
		{
			AllOptions.Swap(i, Index);
		}
	}

	// Return the top 'Count' items
	TArray<EUpgradeType> Result;
	for (int32 i = 0; i < Count && i < AllOptions.Num(); i++)
	{
		Result.Add(AllOptions[i]);
	}
	return Result;
}

FUpgradeData AAutoSurvivorCharacter::GetUpgradeInfo(EUpgradeType Type)
{
	FUpgradeData Info;
	Info.UpgradeType = Type;

	switch (Type)
	{
	case EUpgradeType::Weapon_Shotgun:
		Info.Title = FText::FromString("Shotgun");
		Info.Description = FText::FromString("Fires 5 bullets in a spread.");
		break;
	case EUpgradeType::Weapon_MachineGun:
		Info.Title = FText::FromString("Machine Gun");
		Info.Description = FText::FromString("High fire rate, low accuracy.");
		break;
	case EUpgradeType::Stat_Health:
		Info.Title = FText::FromString("Vitality");
		Info.Description = FText::FromString("Increase Max Health by +50.");
		break;
	case EUpgradeType::Stat_Speed:
		Info.Title = FText::FromString("Haste");
		Info.Description = FText::FromString("Move 20% Faster.");
		break;
	case EUpgradeType::Stat_Damage:
		Info.Title = FText::FromString("Might");
		Info.Description = FText::FromString("Deal 20% more damage.");
		break;
	}
	return Info;
}

void AAutoSurvivorCharacter::ApplyUpgrade(EUpgradeType UpgradeType)
{
	switch (UpgradeType)
	{
	case EUpgradeType::Weapon_Shotgun:
		SetWeapon(EWeaponType::Shotgun);
		break;
	case EUpgradeType::Weapon_MachineGun:
		SetWeapon(EWeaponType::MachineGun);
		break;

	case EUpgradeType::Stat_Health:
		MaxHealth += 50.0f;
		CurrentHealth += 50.0f;
		break;

	case EUpgradeType::Stat_Speed:
		MoveSpeedMultiplier += 0.2f;
		GetCharacterMovement()->MaxWalkSpeed = 600.0f * MoveSpeedMultiplier;
		break;

	case EUpgradeType::Stat_Damage:
		DamageMultiplier += 0.2f;
		break;
	}
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
		ShowLevelUpMenu();
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