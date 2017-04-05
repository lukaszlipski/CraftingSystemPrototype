// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PickupObject.h"
#include "PickupCommon.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class ECommonType : uint8
{
	Alcohol 				UMETA(DisplayName = "Alcohol"),
	Electrics  				UMETA(DisplayName = "Electrics"),
	Scrap					UMETA(DisplayName = "Scrap"),
	Chemicals				UMETA(DisplayName = "Chemicals"),
	MinaeralsAndElements	UMETA(DisplayName = "MinaeralsAndElements"),
	Biowaste				UMETA(DisplayName = "Biowaste")
};

/**
 * 
 */
UCLASS()
class CRAFTING_API APickupCommon : public APickupObject
{
	GENERATED_BODY()
	
public:
	APickupCommon();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type")
	ECommonType ObjectType;
	
};
