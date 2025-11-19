// Fill out your copyright notice in the Description page of Project Settings.

#include "Bullet.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "EnemyCharacter.h" // Include this so we can talk to the Enemy

// Sets default values
ABullet::ABullet()
{
	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// 1. Create Collision Sphere
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(20.0f);
	CollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // Make sure it overlaps!
	RootComponent = CollisionComp;

	// 2. Create Visual Mesh
	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	BulletMesh->SetupAttachment(CollisionComp);
	BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 3. Create Projectile Movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	InitialLifeSpan = 3.0f;
}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();

	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;

	// BIND THE COLLISION EVENT
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ABullet::OnOverlap);
}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// THIS RUNS WHEN WE HIT SOMETHING
void ABullet::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Did we hit an Enemy?
	if (OtherActor && OtherActor != this)
	{
		// Try to cast the actor we hit to an AEnemyCharacter
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(OtherActor);

		if (Enemy)
		{
			// It IS an enemy! Hurt them!
			Enemy->DealDamage(Damage);

			// Destroy the bullet (so it doesn't keep flying through 100 enemies)
			Destroy();
		}
	}
}