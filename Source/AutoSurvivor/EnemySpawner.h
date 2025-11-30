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

	// --- CHANGED: NOW A LIST OF ENEMIES ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<TSubclassOf<AEnemyCharacter>> EnemyTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float BaseSpawnInterval = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float MinSpawnInterval = 0.1f;

	FTimerHandle SpawnTimerHandle;

	float CurrentTime = 0.0f;
	float CurrentDifficulty = 1.0f;

	void SpawnEnemy();
	FVector GetRandomPointOffScreen();
};