// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS()
class AUTOSURVIVOR_API ABullet : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABullet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// The collision sphere (root component)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet")
	USphereComponent* CollisionComp;

	// The visual mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet")
	UStaticMeshComponent* BulletMesh;

	// The component that handles movement (flying)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet")
	UProjectileMovementComponent* ProjectileMovement;

	// How fast the bullet flies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	float Speed = 1000.0f;
};