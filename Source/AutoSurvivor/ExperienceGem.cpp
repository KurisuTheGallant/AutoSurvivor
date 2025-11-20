// Fill out your copyright notice in the Description page of Project Settings.

#include "ExperienceGem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AutoSurvivorCharacter.h" 
#include "Kismet/KismetMathLibrary.h"

AExperienceGem::AExperienceGem()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. Setup Pickup Sphere (Small, for collision)
	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->InitSphereRadius(20.0f);
	PickupSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = PickupSphere;

	// 2. Setup Magnet Sphere (Big, for detection)
	MagnetSphere = CreateDefaultSubobject<USphereComponent>(TEXT("MagnetSphere"));
	MagnetSphere->SetupAttachment(RootComponent);
	MagnetSphere->InitSphereRadius(150.0f); // Default range
	MagnetSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	// 3. Setup Mesh
	GemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GemMesh"));
	GemMesh->SetupAttachment(PickupSphere);
	GemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AExperienceGem::BeginPlay()
{
	Super::BeginPlay();

	// Bind Events
	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AExperienceGem::OnPickupOverlap);
	MagnetSphere->OnComponentBeginOverlap.AddDynamic(this, &AExperienceGem::OnMagnetOverlap);
}

void AExperienceGem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If magnetized, fly to player
	if (bIsMagnetized && PlayerTarget)
	{
		FVector CurrentLoc = GetActorLocation();
		FVector TargetLoc = PlayerTarget->GetActorLocation();

		// Simple interpolation to make it fly smoothly
		// (Or you can just move straight there)
		FVector NewLoc = FMath::VInterpConstantTo(CurrentLoc, TargetLoc, DeltaTime, FlySpeed);

		SetActorLocation(NewLoc);

		// Optional: Increase speed over time so it catches up if you run away
		FlySpeed += 500.0f * DeltaTime;
	}
}

// Triggered when the player enters the large radius
void AExperienceGem::OnMagnetOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Only trigger once
	if (bIsMagnetized) return;

	if (OtherActor && OtherActor->IsA(AAutoSurvivorCharacter::StaticClass()))
	{
		PlayerTarget = OtherActor;
		bIsMagnetized = true;
	}
}

// Triggered when the Gem actually hits the player
void AExperienceGem::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAutoSurvivorCharacter* Player = Cast<AAutoSurvivorCharacter>(OtherActor);

	if (Player)
	{
		Player->AddExperience(XPValue);
		Destroy();
	}
}