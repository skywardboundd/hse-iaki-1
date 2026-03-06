// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameZone.h"
#include "Components/BoxComponent.h"

AGameZone::AGameZone()
{
	PrimaryActorTick.bCanEverTick = false;

	VerticalLine = CreateDefaultSubobject<UBoxComponent>(TEXT("VerticalLine"));
	RootComponent = VerticalLine;
	VerticalLine->SetBoxExtent(FVector(15.0f, 15.0f, 500.0f));
	VerticalLine->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VerticalLine->SetCollisionObjectType(ECC_WorldDynamic);
	VerticalLine->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void AGameZone::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (VerticalLine)
	{
		VerticalLine->SetBoxExtent(FVector(15.0f, 15.0f, LineHeight));
	}
}
