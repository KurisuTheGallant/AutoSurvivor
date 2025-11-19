// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Configure Movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerTarget = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

// Called every frame
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

// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// --- HEALTH LOGIC ---

void AEnemyCharacter::DealDamage(float Amount)
{
	Health -= Amount;

	if (Health <= 0.0f)
	{
		Destroy(); // Goodbye!
	}
}