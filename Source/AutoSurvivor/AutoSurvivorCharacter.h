// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "InputActionValue.h"
#include "AutoSurvivorCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class ABullet;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

// --- NEW: Weapon Enum ---
// This defines the list of weapons our game knows about.
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Pistol UMETA(DisplayName = "Pistol"),
	Shotgun UMETA(DisplayName = "Shotgun"),
	MachineGun UMETA(DisplayName = "Machine Gun")
};

UCLASS(config = Game)
class AAutoSurvivorCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:
	AAutoSurvivorCharacter();

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void FireWeapon();
	FTimerHandle FireTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<ABullet> BulletClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float FireRate = 0.5f;

	// --- NEW: Current Weapon Tracking ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	EWeaponType CurrentWeapon = EWeaponType::Pistol;

	// Helper to switch weapons internally
	void SetWeapon(EWeaponType NewWeapon);

	// --- UI INTERACTION ---

	// Called by the UI when a player clicks a card
	UFUNCTION(BlueprintCallable, Category = "Level Up")
	void ApplyUpgrade(EWeaponType NewWeapon);

	// Event triggered when XP hits max. Implemented in Blueprint to open WBP_LevelUpMenu.
	UFUNCTION(BlueprintImplementableEvent, Category = "Level Up")
	void ShowLevelUpMenu();

	// --- GAME FEEL ---
	UFUNCTION(BlueprintImplementableEvent)
	void PlayShootEffects();

	AActor* GetNearestEnemy();

	// --- LEVELING SYSTEM ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Experience")
	float CurrentExperience = 0.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Experience")
	float MaxExperience = 100.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Experience")
	int32 CurrentLevel = 1;
	void AddExperience(float Amount);

	// --- HEALTH & DEATH SYSTEM ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.0f;
	bool bIsDead = false;
	void DamagePlayer(float Amount);
	UFUNCTION(BlueprintImplementableEvent)
	void OnDeath();
};