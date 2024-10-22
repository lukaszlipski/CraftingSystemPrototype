// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "Components/WidgetComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "PickupObject.h"
#include "craftingCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FItemsDelegate);


USTRUCT(BlueprintType)
struct FPickupItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		TSubclassOf<class APickupObject> Class;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		int Number;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		bool IsRare;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		FString SName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		FString Description;
};

USTRUCT(BlueprintType)
struct FItemImages
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		TSubclassOf<class APickupObject> Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		UTexture2D *Texture;

};


UCLASS(config=Game)
class AcraftingCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* VR_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* L_MotionController;

public:
	AcraftingCharacter();

protected:
	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds);

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AcraftingProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

protected:
	
	/** Fires a projectile. */
	void OnFire();
	void OnStopFire();

	/** Resets HMD orientation and position in VR. */
	void OnResetVR();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);


	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;

	// Mouse movement
	void AddControllerYawInput(float Val);
	void AddControllerPitchInput(float Val);

	APlayerController *PlayerController;

	// -------------- Inventory & pickups ---------------
	void SetInventory();
	bool bIsInventoryOpen;
	bool bIsUIRotting;
	bool bIsCraftingTableCurrentUI;
	float currentAngle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UI)
	class UWidgetComponent* PlayerInventory;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UI)
	class UWidgetComponent* RecipeList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UI)
	float UIRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UI)
	float UISpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UI)
	class UWidgetInteractionComponent* InteractionPointer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	TArray<FPickupItem> CurrentItems;

	UPROPERTY(BlueprintAssignable, Category = UI)
	FItemsDelegate Callback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI)
	FRotator UIInitRotation;

	FRotator UICurrentRotation;

public:

	UFUNCTION(BlueprintCallable, Category = UI)
		int IncreaseItemNumber(APickupObject *po);

	UFUNCTION(BlueprintCallable, Category = UI)
		int IncreaseItemNumberS(FPickupItem po);

	UFUNCTION(BlueprintCallable, Category = UI)
		int DecreaseItemNumber(APickupObject *po);

	UFUNCTION(BlueprintCallable, Category = UI)
		int DecreaseItemNumberS(FPickupItem po);

	UFUNCTION(BlueprintCallable, Category = UI)
		void SwitchToRecipeList();

	UFUNCTION(BlueprintCallable, Category = UI)
		void SwitchToCraftingTable();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = UI)
		bool GetIsInventoryOpen();
	// ------------------------------------------------

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

