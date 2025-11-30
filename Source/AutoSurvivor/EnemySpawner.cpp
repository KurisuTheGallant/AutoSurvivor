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

	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, BaseSpawnInterval, true);
}

void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentTime += DeltaTime;
	CurrentDifficulty = 1.0f + (CurrentTime / 60.0f);
}

void AEnemySpawner::SpawnEnemy()
{
	// Safety Check: Do we have any enemies in the list?
	if (EnemyTypes.Num() == 0) return;

	FVector SpawnLocation = GetRandomPointOffScreen();
	FRotator SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// --- PICK A RANDOM ENEMY TYPE ---
	int32 RandomIndex = FMath::RandRange(0, EnemyTypes.Num() - 1);
	TSubclassOf<AEnemyCharacter> SelectedEnemyClass = EnemyTypes[RandomIndex];

	if (SelectedEnemyClass)
	{
		AEnemyCharacter* NewEnemy = GetWorld()->SpawnActor<AEnemyCharacter>(SelectedEnemyClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (NewEnemy)
		{
			NewEnemy->SetStats(CurrentDifficulty);
		}
	}

	float NewInterval = BaseSpawnInterval / CurrentDifficulty;
	if (NewInterval < MinSpawnInterval)
	{
		NewInterval = MinSpawnInterval;
	}

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