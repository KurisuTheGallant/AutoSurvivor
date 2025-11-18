// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemySpawner.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyCharacter.h"
#include "GameFramework/Character.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// Create the box component (just for visualization in editor, if needed)
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	RootComponent = SpawnArea;
	SpawnArea->SetBoxExtent(FVector(1000.f, 1000.f, 100.f));
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	// Start the timer to call "SpawnEnemy" every "SpawnInterval" seconds, looping = true
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, SpawnInterval, true);
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemySpawner::SpawnEnemy()
{
	if (!EnemyClass) return; // Safety check: Did we forget to set the blueprint?

	FVector SpawnLocation = GetRandomPointOffScreen();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<AEnemyCharacter>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
}

FVector AEnemySpawner::GetRandomPointOffScreen()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!PlayerCharacter) return FVector::ZeroVector;

	FVector PlayerLocation = PlayerCharacter->GetActorLocation();

	// --- SURVIVOR SPAWN LOGIC ---
	// We want to spawn enemies in a circle AROUND the player.
	// Radius should be slightly larger than the screen view (e.g., 1200-1500 units).

	float MinDistance = 1200.0f;
	float MaxDistance = 1600.0f;

	float RandomDistance = FMath::RandRange(MinDistance, MaxDistance);

	// Pick a random angle in radians (0 to 2*PI)
	float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);

	// Convert Angle/Distance to X/Y offsets
	float OffsetX = RandomDistance * FMath::Cos(RandomAngle);
	float OffsetY = RandomDistance * FMath::Sin(RandomAngle);

	FVector SpawnPos = PlayerLocation + FVector(OffsetX, OffsetY, 0.0f);

	// Ensure they spawn at the player's Z height so they don't fall through the floor or spawn in the sky
	SpawnPos.Z = PlayerLocation.Z;

	return SpawnPos;
}