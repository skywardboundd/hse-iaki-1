// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AMazeGenerator.generated.h"

UCLASS()
class DIIKSTRA_API AAMazeGenerator : public AActor
{
	GENERATED_BODY()

public:
	AAMazeGenerator();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze", meta = (ClampMin = 3, ClampMax = 50))
	int32 MazeWidth = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze", meta = (ClampMin = 3, ClampMax = 50))
	int32 MazeHeight = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze", meta = (ClampMin = 50, ClampMax = 500))
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze", meta = (ClampMin = 50, ClampMax = 500))
	float WallHeight = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maze")
	TSubclassOf<AActor> WallClass;

private:
	void GenerateMaze();

	void CarvePassages(int32 X, int32 Y);

	void SpawnWall(int32 GridX, int32 GridY);

	TArray<TArray<bool>> Grid;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedWalls;
};
