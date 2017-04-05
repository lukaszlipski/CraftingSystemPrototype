// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PickupObject.generated.h"

UCLASS()
class CRAFTING_API APickupObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupObject();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
		UTexture2D* Texture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
		class UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
		class UCapsuleComponent* Shape;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Type")
		bool bIsRare;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Type")
		bool IsRare() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
