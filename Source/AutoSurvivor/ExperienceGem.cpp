// Fill out your copyright notice in the Description page of Project Settings.

#include "ExperienceGem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AutoSurvivorCharacter.h" // Need this to talk to player

AExperienceGem::AExperienceGem()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. Setup Sphere
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->InitSphereRadius(30.0f);
	SphereComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = SphereComp;

	// 2. Setup Mesh
	GemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GemMesh"));
	GemMesh->SetupAttachment(SphereComp);
	GemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AExperienceGem::BeginPlay()
{
	Super::BeginPlay();

	// Listen for overlaps
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AExperienceGem::OnOverlap);
}

void AExperienceGem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Optional: Add a simple rotation here later to make it spin
}

void AExperienceGem::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the thing we hit is the Player
	AAutoSurvivorCharacter* Player = Cast<AAutoSurvivorCharacter>(OtherActor);

	if (Player)
	{
		// Give XP
		Player->AddExperience(XPValue);

		// Destroy Gem
		Destroy();
	}
}