// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ExperienceGem.h"
#include "AutoSurvivorCharacter.h" // Needed to access Player Health

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

// --- DAMAGE PLAYER ON TOUCH ---
// Unreal automatically calls this when the enemy touches something
void AEnemyCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// Did we touch the player?
	if (OtherActor && OtherActor->IsA(AAutoSurvivorCharacter::StaticClass()))
	{
		AAutoSurvivorCharacter* Player = Cast<AAutoSurvivorCharacter>(OtherActor);
		if (Player)
		{
			// Deal 10 damage
			Player->DamagePlayer(10.0f);

			// Optional: Destroy enemy so they don't deal damage 100 times a second
			// Or implement a "Cooldown" system. For now, suicide-attack is easiest.
			Destroy();
		}
	}
}