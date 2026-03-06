// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Set.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "GridPathfinder.h"
#include "EnvironmentGridScanner.generated.h"

class UProceduralMeshComponent;

UENUM(BlueprintType)
enum class EPathfindingAlgorithm : uint8
{
	Dijkstra UMETA(DisplayName = "Dijkstra"),
	AStar UMETA(DisplayName = "A*")
};

UCLASS()
class DIIKSTRA_API AEnvironmentGridScanner : public AActor
{
	GENERATED_BODY()

public:
	AEnvironmentGridScanner();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = 1, ClampMax = 200))
	int32 GridWidth = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = 1, ClampMax = 200))
	int32 GridHeight = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = 10, ClampMax = 500))
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = 50, ClampMax = 1000))
	float TraceHeight = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	bool bIgnoreSelf = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	bool bDynamicScan = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = 0.0, ClampMax = 5.0, EditCondition = "bDynamicScan"))
	float ScanInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (ClampMin = 0.0, ClampMax = 5.0))
	float InitialScanDelay = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
	bool bShowGridVisualization = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization")
	TObjectPtr<UMaterialInterface> GridMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visualization", meta = (ClampMin = 0, ClampMax = 100))
	float GridVisualizationHeight = 5.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	TArray<uint8> Grid;

	UPROPERTY(BlueprintReadOnly, Category = "Grid", meta = (DisplayName = "Base Grid (Initial Scan)"))
	TArray<uint8> GridBase;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void ScanEnvironment();

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsWall(int32 X, int32 Y) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsPassable(int32 X, int32 Y) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector GetCellWorldCenter(int32 X, int32 Y) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	void WorldToGrid(const FVector& WorldPos, int32& OutX, int32& OutY) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsValidCell(int32 X, int32 Y) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	int32 GetCellValue(int32 X, int32 Y) const;

	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	FIntPoint StartCell = FIntPoint(-1, -1);

	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	FIntPoint FinishCell = FIntPoint(-1, -1);

	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector GetStartWorldLocation() const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector GetFinishWorldLocation() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TSubclassOf<AActor> ZoneClass;

	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	void FindPath();

	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	void FindPathWithAlgorithm(EPathfindingAlgorithm Algorithm);

	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	void StartPathfindingAnimation();

	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	void StartPathfindingAnimationWithAlgorithm(EPathfindingAlgorithm Algorithm);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
	EPathfindingAlgorithm DefaultAlgorithm = EPathfindingAlgorithm::Dijkstra;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
	TArray<float> PathWeights;

	UPROPERTY(BlueprintReadOnly, Category = "Pathfinding")
	TArray<FIntPoint> PathResult;

	UPROPERTY(BlueprintReadOnly, Category = "Pathfinding")
	TArray<FIntPoint> ExploredOrder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding", meta = (ClampMin = 0.01, ClampMax = 1.0))
	float AnimationStepInterval = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
	bool bEnablePathfindingKeys = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding", meta = (EditCondition = "bEnablePathfindingKeys"))
	FKey DijkstraKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding", meta = (EditCondition = "bEnablePathfindingKeys"))
	FKey AStarKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding", meta = (ClampMin = 0.0, ClampMax = 5.0, EditCondition = "bEnablePathfindingKeys"))
	float PathfindingKeyDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
	bool bAutoRunPathfinding = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding", meta = (ClampMin = 0.0, ClampMax = 10.0, EditCondition = "bAutoRunPathfinding"))
	float AutoRunPathfindingDelay = 2.0f;

private:
	bool SampleCell(int32 X, int32 Y);

	bool SampleCellDynamicOnly(int32 X, int32 Y);

	void BuildGridMesh();

	void UpdateStartFinishCells();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UProceduralMeshComponent> GridMeshComponent;

	float TimeSinceLastScan = 0.0f;
	bool bInitialScanDone = false;

	FTimerHandle InitialScanTimerHandle;
	void OnInitialScanTimer();

	TObjectPtr<AActor> CachedStartZone;
	TObjectPtr<AActor> CachedFinishZone;

	TArray<TObjectPtr<AActor>> ZoneActorsToIgnore;

	int32 AnimationStepIndex = 0;
	FTimerHandle PathAnimationTimerHandle;
	FTimerHandle PathfindingKeyDelayTimerHandle;
	FTimerHandle AutoRunPathfindingTimerHandle;
	void OnPathAnimationStep();
	void OnPathfindingKeyDelayElapsed();
	void OnAutoRunPathfindingDelayElapsed();
	void UpdatePathVisualization();
	FGridPathfinder::FResult RunPathfinding(EPathfindingAlgorithm Algorithm) const;

	TSet<FIntPoint> VisualizedExploredCells;
	TSet<FIntPoint> VisualizedPathCells;
	TSet<FIntPoint> VisualizedOpenSetCells;
	TArray<TSet<FIntPoint>> OpenSetPerStep;

	EPathfindingAlgorithm PendingAlgorithmForKey = EPathfindingAlgorithm::Dijkstra;
	bool bDijkstraKeyWasPressed = false;
	bool bAStarKeyWasPressed = false;
};
