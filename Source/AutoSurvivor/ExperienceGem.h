// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExperienceGem.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class AUTOSURVIVOR_API AExperienceGem : public AActor
{
	GENERATED_BODY()

public:
	AExperienceGem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// The small sphere for actually collecting the item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gem")
	USphereComponent* PickupSphere;

	// The large sphere that triggers the magnet effect
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gem")
	USphereComponent* MagnetSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gem")
	UStaticMeshComponent* GemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gem")
	float XPValue = 20.0f;

	// --- MAGNET LOGIC ---

	bool bIsMagnetized = false;
	AActor* PlayerTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gem")
	float FlySpeed = 800.0f; // Speed when flying to player

	// Functions
	UFUNCTION()
	void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnMagnetOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};