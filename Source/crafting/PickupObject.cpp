// Fill out your copyright notice in the Description page of Project Settings.

#include "crafting.h"
#include "craftingCharacter.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "PickupObject.h"

// Sets default values
APickupObject::APickupObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsRare = false;

	Shape = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision Shape"));
	Shape->SetupAttachment(RootComponent);
	Shape->bGenerateOverlapEvents = true;
	Shape->OnComponentBeginOverlap.AddDynamic(this, &APickupObject::OnOverlapBegin);


	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Shape);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->bGenerateOverlapEvents = false;
}

bool APickupObject::IsRare() const
{
	return bIsRare;
}

void APickupObject::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	APawn *playerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (this->IsValidLowLevelFast() && OtherActor != nullptr && OtherActor != this && OtherActor->GetClass() == playerPawn->GetClass())
	{
		int number = Cast<AcraftingCharacter>(playerPawn)->IncreaseItemNumber(this);
		this->Destroy();
	}
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

