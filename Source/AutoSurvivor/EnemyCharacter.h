// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class AUTOSURVIVOR_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class ACharacter* PlayerTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MovementSpeed = 300.0f;

	// --- HEALTH SYSTEM ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health = 100.0f;

	// Function to apply damage to this enemy
	void DealDamage(float Amount);
};