// Copyright Epic Games, Inc. All Rights Reserved.

#include "MazeWall.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AMazeWall::AMazeWall()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(FName("BlockAll"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMeshFinder.Object);
	}
}

void AMazeWall::BeginPlay()
{
	Super::BeginPlay();

	if (!Mesh->GetStaticMesh())
	{
		const TCHAR* Paths[] = {
			TEXT("/Engine/BasicShapes/Cube.Cube"),
			TEXT("/Engine/Engine/BasicShapes/Cube.Cube"),
			TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube")
		};
		for (const TCHAR* Path : Paths)
		{
			if (UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, Path))
			{
				Mesh->SetStaticMesh(CubeMesh);
				break;
			}
		}
	}
}
