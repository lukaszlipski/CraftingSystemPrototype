// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "crafting.h"
#include "craftingCharacter.h"
#include "craftingProjectile.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "Kismet/KismetMathLibrary.h"
#include "MotionControllerComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AcraftingCharacter

AcraftingCharacter::AcraftingCharacter()
{

	bIsInventoryOpen = false;
	bIsUIRotting = false;
	bIsCraftingTableCurrentUI = true;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Player inventory setup
	PlayerInventory = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerInventory"));
	PlayerInventory->SetupAttachment(FirstPersonCameraComponent);
	PlayerInventory->bGenerateOverlapEvents = false;

	// Player recipe list
	RecipeList = CreateDefaultSubobject<UWidgetComponent>(TEXT("RecipeList"));
	RecipeList->SetupAttachment(FirstPersonCameraComponent);
	RecipeList->bGenerateOverlapEvents = false;

	InteractionPointer = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("InteractionPointer"));
	InteractionPointer->SetupAttachment(FirstPersonCameraComponent);
	
	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->Hand = EControllerHand::Right;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AcraftingCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// set player controller var
	PlayerController = Cast<APlayerController>(GetController());
	
	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}
	PlayerInventory->SetVisibility(false);
	InteractionPointer->Deactivate();

	// rotate recipe table 90 degrees
	FVector fpsCamPos = FirstPersonCameraComponent->GetComponentToWorld().GetLocation();
	FVector recipePos = RecipeList->GetComponentToWorld().GetLocation();
	FVector dirFR = recipePos - fpsCamPos;
	float distanceFR = FVector::Dist(recipePos, fpsCamPos);
	dirFR.Normalize();
	FVector newPosFR = UKismetMathLibrary::RotateAngleAxis(dirFR, 90, GetCapsuleComponent()->GetUpVector());
	RecipeList->SetWorldLocation(FirstPersonCameraComponent->GetComponentToWorld().GetLocation() + newPosFR * distanceFR);

	// Get current rotation of crafting table
	UICurrentRotation = UIInitRotation = PlayerInventory->GetRelativeTransform().GetRotation().Rotator();
}

void AcraftingCharacter::Tick(float DeltaSeconds)
{
	// Call the base class  
	Super::Tick(DeltaSeconds);

	if (bIsInventoryOpen)
	{
		// Rotate interaction pointer
		FHitResult hit;
		PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, hit);
		FVector end = hit.Location;
		FVector start = FirstPersonCameraComponent->GetComponentToWorld().GetLocation();
		FQuat newRotation = FRotationMatrix::MakeFromX(end - start).Rotator().Quaternion();
		InteractionPointer->SetWorldRotation(newRotation);

		// Get mouse delta
		FVector2D centerViewPort;
		GetWorld()->GetGameViewport()->GetViewportSize(centerViewPort);
		centerViewPort /= 2;
		FVector2D MouseDelta;
		PlayerController->GetMousePosition(MouseDelta.X, MouseDelta.Y);
		MouseDelta = (MouseDelta - centerViewPort) / centerViewPort;

		// Rotate player inventory
		FRotator CameraRotation;
		CameraRotation.Pitch = UIInitRotation.Pitch + (MouseDelta.Y * 7);
		CameraRotation.Yaw = UIInitRotation.Yaw + (MouseDelta.X * 5);
		CameraRotation.Roll = UIInitRotation.Roll;
		UICurrentRotation = FMath::RInterpTo(UICurrentRotation, CameraRotation, DeltaSeconds, 1.0f);
		if (bIsCraftingTableCurrentUI)
		{
			PlayerInventory->SetRelativeRotation(UICurrentRotation);
		}
		else
		{
			RecipeList->SetRelativeRotation(UICurrentRotation);
		}
	
	}

	// Rotate UI
	if (bIsUIRotting)
	{
		FVector fpsCamPos = FirstPersonCameraComponent->GetComponentToWorld().GetLocation();
		FVector recipePos = RecipeList->GetComponentToWorld().GetLocation();
		FVector craftPos = PlayerInventory->GetComponentToWorld().GetLocation();
		FVector dirFR = recipePos - fpsCamPos;
		FVector dirFC = craftPos - fpsCamPos;
		float distanceFR = FVector::Dist(recipePos, fpsCamPos);
		float distanceFC = FVector::Dist(craftPos, fpsCamPos);
		dirFR.Normalize();
		dirFC.Normalize();

		// set location
		FVector newPosFR = UKismetMathLibrary::RotateAngleAxis(dirFR, DeltaSeconds * UISpeed, FirstPersonCameraComponent->GetUpVector());
		RecipeList->SetWorldLocation(fpsCamPos + newPosFR * distanceFR);
		FVector newPosFC = UKismetMathLibrary::RotateAngleAxis(dirFC, DeltaSeconds * UISpeed, FirstPersonCameraComponent->GetUpVector());
		PlayerInventory->SetWorldLocation(fpsCamPos + newPosFC * distanceFC);
		currentAngle += DeltaSeconds * UISpeed;



		if (bIsCraftingTableCurrentUI)
		{
			if (currentAngle >= 0)
			{
				bIsUIRotting = false;
			}
		}
		else
		{
			if (currentAngle <= -90)
			{
				bIsUIRotting = false;
			}
		}

	} 

}

//////////////////////////////////////////////////////////////////////////
// Input

int AcraftingCharacter::IncreaseItemNumber(APickupObject * po)
{
	for (int i = 0; i < CurrentItems.Num(); i++)
	{
		if (CurrentItems[i].Class == po->GetClass())
		{
			++CurrentItems[i].Number;
			Callback.Broadcast();
			return CurrentItems[i].Number;
		}
	}

	CurrentItems.Add(FPickupItem{ po->GetClass(),1,po->IsRare() });
	Callback.Broadcast();
	return 1;
}

int AcraftingCharacter::IncreaseItemNumberS(FPickupItem po)
{
	for (int i = 0; i < CurrentItems.Num(); i++)
	{
		if (CurrentItems[i].Class == po.Class)
		{
			++CurrentItems[i].Number;
			Callback.Broadcast();
			return CurrentItems[i].Number;
		}
	}
	po.Number = 1;
	CurrentItems.Add(po);
	Callback.Broadcast();
	return 1;
}

int AcraftingCharacter::DecreaseItemNumber(APickupObject * po)
{
	for (int i = 0; i < CurrentItems.Num(); i++)
	{
		if (CurrentItems[i].Class == po->GetClass())
		{
			if (CurrentItems[i].Number > 1)
			{
				--CurrentItems[i].Number;
				Callback.Broadcast();
				return CurrentItems[i].Number;
			}
			else
			{
				CurrentItems.RemoveAt(i);
				Callback.Broadcast();
				return 0;
			}
		}
	}
	return 0;
}

int AcraftingCharacter::DecreaseItemNumberS(FPickupItem po)
{
	for (int i = 0; i < CurrentItems.Num(); i++)
	{
		if (CurrentItems[i].Class == po.Class)
		{
			if (CurrentItems[i].Number > 1)
			{
				--CurrentItems[i].Number;
				Callback.Broadcast();
				return CurrentItems[i].Number;
			}
			else
			{
				CurrentItems.RemoveAt(i);
				Callback.Broadcast();
				return 0;
			}
		}
	}
	return 0;
}

void AcraftingCharacter::SwitchToRecipeList()
{
	if (currentAngle <= -90)
	{
		bIsUIRotting = false;
		return;
	}
	bIsUIRotting = true;
	if (bIsCraftingTableCurrentUI)
	{
		UISpeed = -UISpeed;
	}
	bIsCraftingTableCurrentUI = false;
}

void AcraftingCharacter::SwitchToCraftingTable()
{
	
	if (currentAngle >= 0)
	{
		bIsUIRotting = false;
		return;
	}
	if (!bIsCraftingTableCurrentUI)
	{
		UISpeed = -UISpeed;
	}
	bIsUIRotting = true;
	bIsCraftingTableCurrentUI = true;
}

void AcraftingCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &AcraftingCharacter::SetInventory);

	//InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AcraftingCharacter::TouchStarted);
	if (EnableTouchscreenMovement(PlayerInputComponent) == false)
	{
		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AcraftingCharacter::OnFire);
		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AcraftingCharacter::OnStopFire);
	}

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AcraftingCharacter::OnResetVR);

	PlayerInputComponent->BindAxis("MoveForward", this, &AcraftingCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AcraftingCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &AcraftingCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AcraftingCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AcraftingCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AcraftingCharacter::LookUpAtRate);
}

void AcraftingCharacter::OnFire()
{
	// try and fire a projectile
	if (!bIsInventoryOpen)
	{
		if (ProjectileClass != NULL)
		{
			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				if (bUsingMotionControllers)
				{
					const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
					const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
					World->SpawnActor<AcraftingProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
				}
				else
				{
					const FRotator SpawnRotation = GetControlRotation();
					// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
					const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

					//Set Spawn Collision Handling Override
					FActorSpawnParameters ActorSpawnParams;
					ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					// spawn the projectile at the muzzle
					World->SpawnActor<AcraftingProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
				}
			}
		}

		// try and play the sound if specified
		if (FireSound != NULL && !bIsInventoryOpen)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		// try and play a firing animation if specified
		if (FireAnimation != NULL)
		{
			// Get the animation object for the arms mesh
			UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
			if (AnimInstance != NULL)
			{
				AnimInstance->Montage_Play(FireAnimation, 1.f);
			}
		}
	}
	else
	{
		InteractionPointer->PressPointerKey(EKeys::LeftMouseButton);
	}
}

void AcraftingCharacter::OnStopFire()
{
	InteractionPointer->ReleasePointerKey(EKeys::LeftMouseButton);
}

void AcraftingCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AcraftingCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AcraftingCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = false;
}

void AcraftingCharacter::SetInventory()
{
	bIsInventoryOpen = !bIsInventoryOpen;

	Mesh1P->SetVisibility(!bIsInventoryOpen, true);
	FP_Gun->SetVisibility(!bIsInventoryOpen, true);

	PlayerController->bShowMouseCursor = bIsInventoryOpen;

	if (bIsInventoryOpen)
	{
		SwitchToCraftingTable();

		FVector2D viewportSize;
		GetWorld()->GetGameViewport()->GetViewportSize(viewportSize);
		PlayerController->SetMouseLocation(viewportSize.X/2.0f,viewportSize.Y/2.0f);
		
		RecipeList->SetVisibility(true);
		PlayerInventory->SetVisibility(true);
		FInputModeGameAndUI mode;
		mode.SetLockMouseToViewport(true);
		mode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(mode);

		InteractionPointer->Activate();
	}
	else
	{
		RecipeList->SetVisibility(false);
		PlayerInventory->SetVisibility(false);
		FInputModeGameOnly mode;
		PlayerController->SetInputMode(mode);

		InteractionPointer->Deactivate();
	}

}

void AcraftingCharacter::AddControllerYawInput(float Val)
{
	if (Val != 0.f && Controller && Controller->IsLocalPlayerController() && !bIsInventoryOpen)
	{
		APlayerController* const PC = CastChecked<APlayerController>(Controller);
		PC->AddYawInput(Val);
	}
}

void AcraftingCharacter::AddControllerPitchInput(float Val)
{
	if (Val != 0.f && Controller && Controller->IsLocalPlayerController() && !bIsInventoryOpen)
	{
		APlayerController* const PC = CastChecked<APlayerController>(Controller);
		PC->AddPitchInput(Val);
	}
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AcraftingCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void AcraftingCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AcraftingCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AcraftingCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AcraftingCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AcraftingCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	bool bResult = false;
	if (FPlatformMisc::GetUseVirtualJoysticks() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		bResult = true;
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AcraftingCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AcraftingCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AcraftingCharacter::TouchUpdate);
	}
	return bResult;
}
