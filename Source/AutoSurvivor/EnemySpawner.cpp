// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemySpawner.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyCharacter.h"
#include "GameFramework/Character.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	RootComponent = SpawnArea;
	SpawnArea->SetBoxExtent(FVector(1000.f, 1000.f, 100.f));
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	// Start the initial timer
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, BaseSpawnInterval, true);
}

void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Increase time
	CurrentTime += DeltaTime;

	// Formula: Every 60 seconds, difficulty increases by +1.0 (100%)
	// 0s = 1.0x
	// 30s = 1.5x
	// 60s = 2.0x
	CurrentDifficulty = 1.0f + (CurrentTime / 60.0f);
}

void AEnemySpawner::SpawnEnemy()
{
	if (!EnemyClass) return;

	FVector SpawnLocation = GetRandomPointOffScreen();
	FRotator SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyCharacter* NewEnemy = GetWorld()->SpawnActor<AEnemyCharacter>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (NewEnemy)
	{
		// BUFF THE ENEMY!
		NewEnemy->SetStats(CurrentDifficulty);
	}

	// --- DYNAMIC SPAWN RATE ---
	// The harder the game, the faster they spawn.
	// Example: At 2.0 difficulty, spawn rate is divided by 2 (twice as fast).
	float NewInterval = BaseSpawnInterval / CurrentDifficulty;

	// Clamp it so it doesn't go to 0.00001 and crash the game
	if (NewInterval < MinSpawnInterval)
	{
		NewInterval = MinSpawnInterval;
	}

	// Reset timer with new speed
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, NewInterval, true);
}

FVector AEnemySpawner::GetRandomPointOffScreen()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter) return FVector::ZeroVector;

	FVector PlayerLocation = PlayerCharacter->GetActorLocation();

	float MinDistance = 1200.0f;
	float MaxDistance = 1600.0f;
	float RandomDistance = FMath::RandRange(MinDistance, MaxDistance);
	float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);

	float OffsetX = RandomDistance * FMath::Cos(RandomAngle);
	float OffsetY = RandomDistance * FMath::Sin(RandomAngle);

	FVector SpawnPos = PlayerLocation + FVector(OffsetX, OffsetY, 0.0f);
	SpawnPos.Z = PlayerLocation.Z;

	return SpawnPos;
}