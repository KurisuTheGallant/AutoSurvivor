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

// --- 1. EXISTING WEAPON ENUM (Kept for internal logic) ---
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Pistol UMETA(DisplayName = "Pistol"),
	Shotgun UMETA(DisplayName = "Shotgun"),
	MachineGun UMETA(DisplayName = "Machine Gun")
};

// --- 2. NEW: UPGRADE TYPE (The Master List) ---
UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	Weapon_Shotgun UMETA(DisplayName = "Weapon: Shotgun"),
	Weapon_MachineGun UMETA(DisplayName = "Weapon: Machine Gun"),
	Stat_Health UMETA(DisplayName = "Stat: Max Health"),
	Stat_Speed UMETA(DisplayName = "Stat: Move Speed"),
	Stat_Damage UMETA(DisplayName = "Stat: Damage Boost")
};

// --- 3. NEW: UPGRADE DATA STRUCT (The Info Card) ---
USTRUCT(BlueprintType)
struct FUpgradeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EUpgradeType UpgradeType;

	// Optional: You can add "UTexture2D* Icon" here later!
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	EWeaponType CurrentWeapon = EWeaponType::Pistol;

	void SetWeapon(EWeaponType NewWeapon);

	// --- UI INTERACTION ---

	// NEW: Helper function to get the Text for an upgrade
	UFUNCTION(BlueprintCallable, Category = "Level Up")
	FUpgradeData GetUpgradeInfo(EUpgradeType Type);

	// NEW: Returns a list of random upgrades for the menu to display
	UFUNCTION(BlueprintCallable, Category = "Level Up")
	TArray<EUpgradeType> GetRandomUpgrades(int32 Count);

	// UPDATED: Now takes the master Enum
	UFUNCTION(BlueprintCallable, Category = "Level Up")
	void ApplyUpgrade(EUpgradeType UpgradeType);

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

	// --- NEW: PASSIVE STATS ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float DamageMultiplier = 1.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float MoveSpeedMultiplier = 1.0f;
};