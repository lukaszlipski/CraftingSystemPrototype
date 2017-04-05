// Fill out your copyright notice in the Description page of Project Settings.

#include "crafting.h"
#include "PickupObject.h"


// Sets default values
APickupObject::APickupObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsRare = false;

	Shape = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision Shape"));
	Shape->SetupAttachment(RootComponent);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Shape);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool APickupObject::IsRare() const
{
	return bIsRare;
}

// Called when the game starts or when spawned
void APickupObject::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APickupObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

