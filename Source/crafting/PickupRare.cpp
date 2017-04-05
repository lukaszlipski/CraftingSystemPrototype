// Fill out your copyright notice in the Description page of Project Settings.

#include "crafting.h"
#include "PickupRare.h"


APickupRare::APickupRare()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsRare = true;

}

