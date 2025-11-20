// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ExperienceGem.h" // Include gem so we can spawn it

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

// --- MODIFIED DAMAGE FUNCTION ---

void AEnemyCharacter::DealDamage(float Amount)
{
	Health -= Amount;

	if (Health <= 0.0f)
	{
		// Spawn the Gem right where the enemy is standing
		if (GemClass)
		{
			FVector SpawnLoc = GetActorLocation();
			// Spawn it slightly lower so it sits on the ground (or adjust collision)
			GetWorld()->SpawnActor<AExperienceGem>(GemClass, SpawnLoc, FRotator::ZeroRotator);
		}

		Destroy();
	}
}