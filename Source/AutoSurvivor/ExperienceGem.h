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

	// Root collision for pickup
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gem")
	USphereComponent* SphereComp;

	// Visual representation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gem")
	UStaticMeshComponent* GemMesh;

	// How much XP this gem gives
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gem")
	float XPValue = 20.0f;

	// Function called when player overlaps
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};