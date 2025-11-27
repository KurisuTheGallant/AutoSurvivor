// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ExperienceGem.h"
#include "AutoSurvivorCharacter.h"

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

void AEnemyCharacter::DealDamage(float Amount)
{
	Health -= Amount;

	// --- GAME FEEL: TRIGGER VISUALS ---
	// We pass the location so particles spawn at the right spot
	OnDamageReceived(Amount, GetActorLocation());

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

void AEnemyCharacter::SetStats(float DifficultyMultiplier)
{
	Health *= DifficultyMultiplier;
	MovementSpeed *= (1.0f + (DifficultyMultiplier * 0.1f));
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

void AEnemyCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (OtherActor && OtherActor->IsA(AAutoSurvivorCharacter::StaticClass()))
	{
		AAutoSurvivorCharacter* Player = Cast<AAutoSurvivorCharacter>(OtherActor);
		if (Player)
		{
			Player->DamagePlayer(10.0f);
			Destroy();
		}
	}
}