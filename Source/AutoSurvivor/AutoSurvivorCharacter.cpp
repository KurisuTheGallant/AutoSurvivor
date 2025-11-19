// Copyright Epic Games, Inc. All Rights Reserved.

#include "AutoSurvivorCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
// Added includes for shooting
#include "Bullet.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyCharacter.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AAutoSurvivorCharacter

AAutoSurvivorCharacter::AAutoSurvivorCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	// --- SURVIVOR STYLE CAMERA SETTINGS ---
	CameraBoom->TargetArmLength = 1200.0f; // High up
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f)); // Look down at 60 degrees
	CameraBoom->bDoCollisionTest = false; // Don't pull in when hitting walls (standard for top-down)
	CameraBoom->bUsePawnControlRotation = false; // FIXED CAMERA: Do not rotate with mouse
	// --------------------------------------

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void AAutoSurvivorCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}

		// --- PERFORMANCE OPTIMIZATION ---
		// Force the game to run in "Unlit" mode automatically for high FPS
		PlayerController->ConsoleCommand("viewmode unlit");
		// --------------------------------
	}

	// --- START AUTO FIRE ---
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AAutoSurvivorCharacter::FireWeapon, FireRate, true);
}

//////////////////////////////////////////////////////////////////////////
// WEAPON LOGIC

AActor* AAutoSurvivorCharacter::GetNearestEnemy()
{
	TArray<AActor*> AllEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyCharacter::StaticClass(), AllEnemies);

	AActor* NearestEnemy = nullptr;
	float NearestDistanceSq = FLT_MAX; // Start with a huge number
	FVector MyLoc = GetActorLocation();

	for (AActor* Enemy : AllEnemies)
	{
		if (Enemy)
		{
			float DistSq = FVector::DistSquared(MyLoc, Enemy->GetActorLocation());
			if (DistSq < NearestDistanceSq)
			{
				NearestDistanceSq = DistSq;
				NearestEnemy = Enemy;
			}
		}
	}
	return NearestEnemy;
}

void AAutoSurvivorCharacter::FireWeapon()
{
	if (!BulletClass) return; // Safety check

	// 1. Find target
	AActor* Target = GetNearestEnemy();
	FRotator SpawnRotation = GetActorRotation(); // Default: Shoot forward

	// 2. If we found an enemy, aim at them
	if (Target)
	{
		FVector Direction = Target->GetActorLocation() - GetActorLocation();
		SpawnRotation = Direction.Rotation();
	}

	// 3. Spawn Bullet
	FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 50.0f); // Start slightly in front of player

	GetWorld()->SpawnActor<ABullet>(BulletClass, SpawnLocation, SpawnRotation);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AAutoSurvivorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAutoSurvivorCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAutoSurvivorCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AAutoSurvivorCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AAutoSurvivorCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}