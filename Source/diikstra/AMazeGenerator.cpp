// Copyright Epic Games, Inc. All Rights Reserved.

#include "AMazeGenerator.h"
#include "MazeWall.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

AAMazeGenerator::AAMazeGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	if (!WallClass)
	{
		WallClass = AMazeWall::StaticClass();
	}
}

void AAMazeGenerator::BeginPlay()
{
	Super::BeginPlay();
	GenerateMaze();
}

void AAMazeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAMazeGenerator::GenerateMaze()
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("AMazeGenerator: GetWorld() is null"));
		return;
	}

	if (!WallClass)
	{
		WallClass = AMazeWall::StaticClass();
		UE_LOG(LogTemp, Log, TEXT("AMazeGenerator: WallClass was null, using AMazeWall"));
	}

	const int32 GridWidth = MazeWidth * 2 + 1;
	const int32 GridHeight = MazeHeight * 2 + 1;

	Grid.SetNum(GridWidth);
	for (int32 X = 0; X < GridWidth; ++X)
	{
		Grid[X].SetNum(GridHeight);
		for (int32 Y = 0; Y < GridHeight; ++Y)
		{
			Grid[X][Y] = true;
		}
	}

	CarvePassages(1, 1);

	Grid[0][1] = false;
	Grid[GridWidth - 1][GridHeight - 2] = false;

	int32 WallsSpawned = 0;
	for (int32 X = 0; X < GridWidth; ++X)
	{
		for (int32 Y = 0; Y < GridHeight; ++Y)
		{
			if (Grid[X][Y])
			{
				SpawnWall(X, Y);
				++WallsSpawned;
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("AMazeGenerator: Spawned %d walls at %s"), WallsSpawned, *GetActorLocation().ToString());
}

void AAMazeGenerator::CarvePassages(int32 X, int32 Y)
{
	const int32 GridWidth = MazeWidth * 2 + 1;
	const int32 GridHeight = MazeHeight * 2 + 1;

	Grid[X][Y] = false;

	const int32 Dx[] = { 0, 2, 0, -2 };
	const int32 Dy[] = { -2, 0, 2, 0 };

	TArray<int32> Order = { 0, 1, 2, 3 };
	for (int32 I = Order.Num() - 1; I > 0; --I)
	{
		const int32 J = FMath::RandRange(0, I);
		Order.Swap(I, J);
	}

	for (int32 I = 0; I < 4; ++I)
	{
		const int32 Nx = X + Dx[Order[I]];
		const int32 Ny = Y + Dy[Order[I]];

		if (Nx > 0 && Nx < GridWidth - 1 && Ny > 0 && Ny < GridHeight - 1 && Grid[Nx][Ny])
		{
			Grid[X + (Nx - X) / 2][Y + (Ny - Y) / 2] = false;
			CarvePassages(Nx, Ny);
		}
	}
}

void AAMazeGenerator::SpawnWall(int32 GridX, int32 GridY)
{
	const FVector Origin = GetActorLocation();
	const int32 GridWidth = MazeWidth * 2 + 1;
	const int32 GridHeight = MazeHeight * 2 + 1;
	const float OffsetX = (GridWidth - 1) * 0.5f * CellSize;
	const float OffsetY = (GridHeight - 1) * 0.5f * CellSize;

	const float WorldX = Origin.X + GridX * CellSize - OffsetX;
	const float WorldY = Origin.Y + GridY * CellSize - OffsetY;
	const float WorldZ = Origin.Z + WallHeight * 0.5f;

	const FVector Location(WorldX, WorldY, WorldZ);
	const FVector Scale(CellSize / 100.0f, CellSize / 100.0f, WallHeight / 100.0f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FRotator Rotation(0.0f, 0.0f, 0.0f);
	FTransform SpawnTransform(Rotation, Location, Scale);

	AActor* Wall = GetWorld()->SpawnActor<AActor>(WallClass, SpawnTransform, SpawnParams);
	if (Wall)
	{
		SpawnedWalls.Add(Wall);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AMazeGenerator: Failed to spawn wall at %d,%d"), GridX, GridY);
	}
}
