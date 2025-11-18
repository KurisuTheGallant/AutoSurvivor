// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// Enemies in Survivor games don't need a controller to possess them to move if we use AddMovementInput directly.
	// But keeping AutoPossessAI is fine for future expansion.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Configure Movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face the direction we are running
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f); // Turn speed
	GetCharacterMovement()->MaxWalkSpeed = 300.0f; // Default speed
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Find the player immediately when we spawn
	PlayerTarget = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	// Set our speed to the variable value
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PlayerTarget)
	{
		// 1. Find the direction to the player
		FVector Direction = PlayerTarget->GetActorLocation() - GetActorLocation();

		// 2. We only care about horizontal movement (X and Y), not Z (Height)
		Direction.Z = 0.0f;

		// 3. Normalize the vector (make it length 1, just a direction)
		Direction.Normalize();

		// 4. Apply movement
		AddMovementInput(Direction);
	}
}

// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}