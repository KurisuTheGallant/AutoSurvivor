// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class UBoxComponent;
class AEnemyCharacter;

UCLASS()
class AUTOSURVIVOR_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	UBoxComponent* SpawnArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AEnemyCharacter> EnemyClass;

	// Base spawn rate (starts slow)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float BaseSpawnInterval = 2.0f;

	// Minimum spawn rate (cap it so we don't crash the PC)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float MinSpawnInterval = 0.1f;

	FTimerHandle SpawnTimerHandle;

	// --- DIFFICULTY VARIABLES ---

	float CurrentTime = 0.0f;
	float CurrentDifficulty = 1.0f;

	void SpawnEnemy();
	FVector GetRandomPointOffScreen();
};