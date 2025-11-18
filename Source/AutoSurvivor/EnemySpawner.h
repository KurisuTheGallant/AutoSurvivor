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
	// Sets default values for this actor's properties
	AEnemySpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// The area where enemies can spawn
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	UBoxComponent* SpawnArea;

	// What class of enemy to spawn (we set this in Blueprint)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TSubclassOf<AEnemyCharacter> EnemyClass;

	// How fast to spawn enemies (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float SpawnInterval = 1.0f;

	// The Timer Handle to manage the repetitive spawning
	FTimerHandle SpawnTimerHandle;

	// The function that actually creates the enemy
	void SpawnEnemy();

	// Helper to find a random point around the player
	FVector GetRandomPointOffScreen();
};