// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ExperienceGem.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	PlayerTarget = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	// Apply the speed (in case we forgot to call SetStats)
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PlayerTarget)
	{
		FVector Direction = PlayerTarget->GetActorLocation() - GetActorLocation();
		Direction.Z = 0.0f;
		Direction.Normalize();
		AddMovementInput(Direction);
	}
}

void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemyCharacter::DealDamage(float Amount)
{
	Health -= Amount;

	if (Health <= 0.0f)
	{
		if (GemClass)
		{
			FVector SpawnLoc = GetActorLocation();
			GetWorld()->SpawnActor<AExperienceGem>(GemClass, SpawnLoc, FRotator::ZeroRotator);
		}
		Destroy();
	}
}

// --- DIFFICULTY LOGIC ---

void AEnemyCharacter::SetStats(float DifficultyMultiplier)
{
	// Health scales linearly (Double difficulty = Double Health)
	Health *= DifficultyMultiplier;

	// Speed scales slowly (so they don't become impossible too fast)
	// Example: At 2x difficulty, speed increases by 10%
	MovementSpeed *= (1.0f + (DifficultyMultiplier * 0.1f));

	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}