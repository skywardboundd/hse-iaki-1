// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnvironmentGridScanner.h"
#include "GameZone.h"
#include "GridPathfinder.h"
#include "GameFramework/PlayerController.h"
#include "ProceduralMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
#include "Materials/MaterialInterface.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"

AEnvironmentGridScanner::AEnvironmentGridScanner()
{
	PrimaryActorTick.bCanEverTick = true;

	DijkstraKey = EKeys::H;
	AStarKey = EKeys::J;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	GridMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GridMesh"));
	GridMeshComponent->SetupAttachment(RootComponent);
	GridMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GridMeshComponent->CastShadow = false;
}

void AEnvironmentGridScanner::BeginPlay()
{
	Super::BeginPlay();

	if (InitialScanDelay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			InitialScanTimerHandle, this, &AEnvironmentGridScanner::OnInitialScanTimer,
			InitialScanDelay, false);
	}
	else
	{
		OnInitialScanTimer();
	}
}

void AEnvironmentGridScanner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InitialScanTimerHandle);
		World->GetTimerManager().ClearTimer(PathAnimationTimerHandle);
		World->GetTimerManager().ClearTimer(PathfindingKeyDelayTimerHandle);
		World->GetTimerManager().ClearTimer(AutoRunPathfindingTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void AEnvironmentGridScanner::OnInitialScanTimer()
{
	bInitialScanDone = true;

	if (!ZoneClass)
	{
		ZoneClass = AGameZone::StaticClass();
	}

	ScanEnvironment();

	if (bShowGridVisualization && Grid.Num() > 0 && GridMaterial)
	{
		BuildGridMesh();
	}

	if (bAutoRunPathfinding && AutoRunPathfindingDelay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AutoRunPathfindingTimerHandle, this, &AEnvironmentGridScanner::OnAutoRunPathfindingDelayElapsed,
			AutoRunPathfindingDelay, false);
	}

	const bool bNeedTick = bDynamicScan || bEnablePathfindingKeys || bAutoRunPathfinding || (bShowGridVisualization && !GridMaterial);
	if (!bNeedTick)
	{
		SetActorTickEnabled(false);
	}
}

void AEnvironmentGridScanner::ScanEnvironment()
{
	if (!GetWorld())
	{
		return;
	}

	UpdateStartFinishCells();

	if (GridBase.Num() == 0)
	{
		GridBase.SetNum(GridWidth * GridHeight);
		for (int32 X = 0; X < GridWidth; ++X)
		{
			for (int32 Y = 0; Y < GridHeight; ++Y)
			{
				GridBase[X + Y * GridWidth] = SampleCell(X, Y) ? 1 : 0;
			}
		}
		UE_LOG(LogTemp, Log, TEXT("EnvironmentGridScanner: Initial scan done, saved to GridBase"));
	}

	Grid = GridBase;

	if (bDynamicScan)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			for (int32 Y = 0; Y < GridHeight; ++Y)
			{
				if (Grid[X + Y * GridWidth] == 0 && SampleCellDynamicOnly(X, Y))
				{
					Grid[X + Y * GridWidth] = 1;
				}
			}
		}
	}

	UpdateStartFinishCells();
}

void AEnvironmentGridScanner::UpdateStartFinishCells()
{
	StartCell = FIntPoint(-1, -1);
	FinishCell = FIntPoint(-1, -1);
	CachedStartZone = nullptr;
	CachedFinishZone = nullptr;
	ZoneActorsToIgnore.Empty();

	if (!GetWorld() || !ZoneClass)
	{
		return;
	}

	for (TActorIterator<AActor> It(GetWorld(), ZoneClass); It; ++It)
	{
		AActor* Zone = *It;
		ZoneActorsToIgnore.Add(Zone);

		if (AGameZone* GameZone = Cast<AGameZone>(Zone))
		{
			int32 CellX, CellY;
			WorldToGrid(Zone->GetActorLocation(), CellX, CellY);

			if (GameZone->ZoneType == EGameZoneType::Start)
			{
				CachedStartZone = Zone;
				StartCell = FIntPoint(CellX, CellY);
			}
			else if (GameZone->ZoneType == EGameZoneType::Finish)
			{
				CachedFinishZone = Zone;
				FinishCell = FIntPoint(CellX, CellY);
			}
		}
	}
}

FVector AEnvironmentGridScanner::GetStartWorldLocation() const
{
	return CachedStartZone ? CachedStartZone->GetActorLocation() : FVector::ZeroVector;
}

FVector AEnvironmentGridScanner::GetFinishWorldLocation() const
{
	return CachedFinishZone ? CachedFinishZone->GetActorLocation() : FVector::ZeroVector;
}

void AEnvironmentGridScanner::FindPath()
{
	FindPathWithAlgorithm(DefaultAlgorithm);
}

FGridPathfinder::FResult AEnvironmentGridScanner::RunPathfinding(EPathfindingAlgorithm Algorithm) const
{
	TArray<float> Weights;
	if (PathWeights.Num() == GridWidth * GridHeight)
	{
		Weights = PathWeights;
	}
	else
	{
		Weights.Init(1.0f, GridWidth * GridHeight);
	}

	switch (Algorithm)
	{
	case EPathfindingAlgorithm::AStar:
		return FGridPathfinder::FindPathAStar(Grid, Weights, GridWidth, GridHeight, StartCell, FinishCell);
	case EPathfindingAlgorithm::Dijkstra:
	default:
		return FGridPathfinder::FindPathDijkstra(Grid, Weights, GridWidth, GridHeight, StartCell, FinishCell);
	}
}

void AEnvironmentGridScanner::FindPathWithAlgorithm(EPathfindingAlgorithm Algorithm)
{
	PathResult.Empty();
	ExploredOrder.Empty();
	VisualizedExploredCells.Empty();
	VisualizedPathCells.Empty();
	VisualizedOpenSetCells.Empty();
	OpenSetPerStep.Empty();

	if (StartCell.X < 0 || FinishCell.X < 0 || Grid.Num() == 0)
	{
		return;
	}

	const FGridPathfinder::FResult Result = RunPathfinding(Algorithm);

	PathResult = Result.Path;
	ExploredOrder = Result.ExploredOrder;
	OpenSetPerStep = Result.OpenSetPerStep;

	if (bShowGridVisualization && GridMaterial)
	{
		VisualizedExploredCells.Append(ExploredOrder);
		VisualizedPathCells.Append(PathResult);
		for (const TSet<FIntPoint>& Step : OpenSetPerStep)
		{
			VisualizedOpenSetCells.Append(Step);
		}
		BuildGridMesh();
	}
}

void AEnvironmentGridScanner::StartPathfindingAnimation()
{
	StartPathfindingAnimationWithAlgorithm(DefaultAlgorithm);
}

void AEnvironmentGridScanner::StartPathfindingAnimationWithAlgorithm(EPathfindingAlgorithm Algorithm)
{
	PathResult.Empty();
	ExploredOrder.Empty();
	VisualizedExploredCells.Empty();
	VisualizedPathCells.Empty();
	VisualizedOpenSetCells.Empty();
	OpenSetPerStep.Empty();

	if (StartCell.X < 0 || FinishCell.X < 0 || Grid.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pathfinding: Start(%d,%d) or Finish(%d,%d) not found, or Grid empty"), StartCell.X, StartCell.Y, FinishCell.X, FinishCell.Y);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Pathfinding: Starting from (%d,%d) to (%d,%d)"), StartCell.X, StartCell.Y, FinishCell.X, FinishCell.Y);

	const FGridPathfinder::FResult Result = RunPathfinding(Algorithm);

	PathResult = Result.Path;
	ExploredOrder = Result.ExploredOrder;
	OpenSetPerStep = Result.OpenSetPerStep;

	if (!Result.bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pathfinding: No path found!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Pathfinding: Path found, %d explored, %d path cells"), ExploredOrder.Num(), PathResult.Num());

	GetWorld()->GetTimerManager().ClearTimer(PathAnimationTimerHandle);
	AnimationStepIndex = 0;
	SetActorTickEnabled(true);

	GetWorld()->GetTimerManager().SetTimer(
		PathAnimationTimerHandle, this, &AEnvironmentGridScanner::OnPathAnimationStep,
		AnimationStepInterval, true);
}

void AEnvironmentGridScanner::OnPathfindingKeyDelayElapsed()
{
	StartPathfindingAnimationWithAlgorithm(PendingAlgorithmForKey);
}

void AEnvironmentGridScanner::OnAutoRunPathfindingDelayElapsed()
{
	StartPathfindingAnimationWithAlgorithm(DefaultAlgorithm);
}

void AEnvironmentGridScanner::OnPathAnimationStep()
{
	VisualizedExploredCells.Empty();
	VisualizedOpenSetCells.Empty();
	for (int32 I = 0; I < FMath::Min(AnimationStepIndex + 1, ExploredOrder.Num()); ++I)
	{
		VisualizedExploredCells.Add(ExploredOrder[I]);
	}
	if (OpenSetPerStep.IsValidIndex(AnimationStepIndex))
	{
		VisualizedOpenSetCells.Append(OpenSetPerStep[AnimationStepIndex]);
	}

	if (AnimationStepIndex >= ExploredOrder.Num() - 1)
	{
		GetWorld()->GetTimerManager().ClearTimer(PathAnimationTimerHandle);
		VisualizedPathCells.Append(PathResult);
	}

	UpdatePathVisualization();
	++AnimationStepIndex;
}

void AEnvironmentGridScanner::UpdatePathVisualization()
{
	if (bShowGridVisualization)
	{
		if (GridMaterial)
		{
			BuildGridMesh();
		}
	}
}

void AEnvironmentGridScanner::BuildGridMesh()
{
	GridMeshComponent->ClearAllMeshSections();

	if (!GridMaterial)
	{
		return;
	}
	UMaterialInterface* Material = GridMaterial;

	GridMeshComponent->SetMaterial(0, Material);

	const float Half = CellSize * 0.45f;
	const float Z = GridVisualizationHeight;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UV0;
	TArray<FLinearColor> VertexColors;

	int32 VertexOffset = 0;

		for (int32 X = 0; X < GridWidth; ++X)
		{
			for (int32 Y = 0; Y < GridHeight; ++Y)
			{
				const FVector Center = GetCellWorldCenter(X, Y) + FVector(0, 0, Z);
				FLinearColor Color;
				if (VisualizedPathCells.Contains(FIntPoint(X, Y)))
				{
					Color = FLinearColor(0, 1, 0.3f, 0.7f);
				}
				else if (VisualizedExploredCells.Contains(FIntPoint(X, Y)))
				{
					Color = FLinearColor(1, 1, 0, 0.6f);
				}
				else if (VisualizedOpenSetCells.Contains(FIntPoint(X, Y)))
				{
					Color = FLinearColor(0.5f, 0.5f, 0.5f, 0.6f);
				}
				else if (StartCell.X == X && StartCell.Y == Y)
				{
					Color = FLinearColor(0, 0.4f, 1, 0.6f);
				}
				else if (FinishCell.X == X && FinishCell.Y == Y)
				{
					Color = FLinearColor(0.2f, 0.6f, 1, 0.6f);
				}
				else if (Grid[X + Y * GridWidth] != 0)
				{
					Color = FLinearColor(1, 0, 0, 0.5f);
				}
				else
				{
					Color = FLinearColor(1, 1, 1, 0.5f);
				}

			const FVector V0 = Center + FVector(-Half, -Half, 0);
			const FVector V1 = Center + FVector( Half, -Half, 0);
			const FVector V2 = Center + FVector( Half,  Half, 0);
			const FVector V3 = Center + FVector(-Half,  Half, 0);

			Vertices.Add(V0);
			Vertices.Add(V1);
			Vertices.Add(V2);
			Vertices.Add(V3);

			Normals.Add(FVector::UpVector);
			Normals.Add(FVector::UpVector);
			Normals.Add(FVector::UpVector);
			Normals.Add(FVector::UpVector);

			UV0.Add(FVector2D(0, 0));
			UV0.Add(FVector2D(1, 0));
			UV0.Add(FVector2D(1, 1));
			UV0.Add(FVector2D(0, 1));

			VertexColors.Add(Color);
			VertexColors.Add(Color);
			VertexColors.Add(Color);
			VertexColors.Add(Color);

			Triangles.Add(VertexOffset + 0);
			Triangles.Add(VertexOffset + 1);
			Triangles.Add(VertexOffset + 2);
			Triangles.Add(VertexOffset + 0);
			Triangles.Add(VertexOffset + 2);
			Triangles.Add(VertexOffset + 3);

			VertexOffset += 4;
		}
	}

	GridMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, VertexColors, TArray<FProcMeshTangent>(), true);
}

void AEnvironmentGridScanner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bEnablePathfindingKeys && bInitialScanDone && GetWorld())
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			const bool bDijkstraPressed = PC->IsInputKeyDown(DijkstraKey);
			if (bDijkstraPressed && !bDijkstraKeyWasPressed)
			{
				bDijkstraKeyWasPressed = true;
				PendingAlgorithmForKey = EPathfindingAlgorithm::Dijkstra;
				if (PathfindingKeyDelay > 0.0f)
				{
					GetWorld()->GetTimerManager().ClearTimer(PathfindingKeyDelayTimerHandle);
					GetWorld()->GetTimerManager().SetTimer(
						PathfindingKeyDelayTimerHandle, this, &AEnvironmentGridScanner::OnPathfindingKeyDelayElapsed,
						PathfindingKeyDelay, false);
				}
				else
				{
					StartPathfindingAnimationWithAlgorithm(PendingAlgorithmForKey);
				}
			}
			if (!bDijkstraPressed)
			{
				bDijkstraKeyWasPressed = false;
			}

			const bool bAStarPressed = PC->IsInputKeyDown(AStarKey);
			if (bAStarPressed && !bAStarKeyWasPressed)
			{
				bAStarKeyWasPressed = true;
				PendingAlgorithmForKey = EPathfindingAlgorithm::AStar;
				if (PathfindingKeyDelay > 0.0f)
				{
					GetWorld()->GetTimerManager().ClearTimer(PathfindingKeyDelayTimerHandle);
					GetWorld()->GetTimerManager().SetTimer(
						PathfindingKeyDelayTimerHandle, this, &AEnvironmentGridScanner::OnPathfindingKeyDelayElapsed,
						PathfindingKeyDelay, false);
				}
				else
				{
					StartPathfindingAnimationWithAlgorithm(PendingAlgorithmForKey);
				}
			}
			if (!bAStarPressed)
			{
				bAStarKeyWasPressed = false;
			}
		}
	}
	

	if (bShowGridVisualization && Grid.Num() > 0 && !GridMaterial)
	{
		const float Half = CellSize * 0.45f;
		const float Z = GridVisualizationHeight;

		for (int32 X = 0; X < GridWidth; ++X)
		{
			for (int32 Y = 0; Y < GridHeight; ++Y)
			{
				const FVector Center = GetCellWorldCenter(X, Y) + FVector(0, 0, Z);
				FColor Color;
				if (VisualizedPathCells.Contains(FIntPoint(X, Y)))
				{
					Color = FColor(0, 255, 80, 220);
				}
				else if (VisualizedExploredCells.Contains(FIntPoint(X, Y)))
				{
					Color = FColor(255, 255, 0, 220);
				}
				else if (VisualizedOpenSetCells.Contains(FIntPoint(X, Y)))
				{
					Color = FColor(128, 128, 128, 220);
				}
				else if (StartCell.X == X && StartCell.Y == Y)
				{
					Color = FColor(0, 100, 255, 220);
				}
				else if (FinishCell.X == X && FinishCell.Y == Y)
				{
					Color = FColor(50, 150, 255, 220);
				}
				else if (Grid[X + Y * GridWidth] != 0)
				{
					Color = FColor(255, 0, 0, 200);
				}
				else
				{
					Color = FColor(255, 255, 255, 200);
				}
				DrawDebugSolidBox(GetWorld(), Center, FVector(Half, Half, 1.0f), Color, false, 0.0f);
			}
		}
	}
}

bool AEnvironmentGridScanner::SampleCell(int32 X, int32 Y)
{
	const FVector CellCenter = GetCellWorldCenter(X, Y);

	const float BoxHeightOffset = 20.0f;
	const FVector BoxCenter = CellCenter + FVector(0, 0, TraceHeight * 0.5f + BoxHeightOffset * 0.5f);
	const FVector BoxExtent(CellSize * 0.5f, CellSize * 0.5f, TraceHeight * 0.5f - BoxHeightOffset * 0.5f);

	FCollisionQueryParams QueryParams;
	if (bIgnoreSelf)
	{
		QueryParams.AddIgnoredActor(this);
	}
	if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr)
	{
		QueryParams.AddIgnoredActor(PlayerPawn);
	}
	for (AActor* Zone : ZoneActorsToIgnore)
	{
		if (Zone)
		{
			QueryParams.AddIgnoredActor(Zone);
		}
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionShape BoxShape;
	BoxShape.SetBox(FVector3f(BoxExtent));

	TArray<FOverlapResult> Overlaps;
	const bool bHasOverlap = GetWorld()->OverlapMultiByObjectType(
		Overlaps, BoxCenter, FQuat::Identity, ObjectParams, BoxShape, QueryParams);

	return bHasOverlap && Overlaps.Num() > 0;
}

bool AEnvironmentGridScanner::SampleCellDynamicOnly(int32 X, int32 Y)
{
	const FVector CellCenter = GetCellWorldCenter(X, Y);

	const float BoxHeightOffset = 20.0f;
	const FVector BoxCenter = CellCenter + FVector(0, 0, TraceHeight * 0.5f + BoxHeightOffset * 0.5f);
	const FVector BoxExtent(CellSize * 0.5f, CellSize * 0.5f, TraceHeight * 0.5f - BoxHeightOffset * 0.5f);

	FCollisionQueryParams QueryParams;
	if (bIgnoreSelf)
	{
		QueryParams.AddIgnoredActor(this);
	}
	if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr)
	{
		QueryParams.AddIgnoredActor(PlayerPawn);
	}
	for (AActor* Zone : ZoneActorsToIgnore)
	{
		if (Zone)
		{
			QueryParams.AddIgnoredActor(Zone);
		}
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionShape BoxShape;
	BoxShape.SetBox(FVector3f(BoxExtent));

	TArray<FOverlapResult> Overlaps;
	const bool bHasOverlap = GetWorld()->OverlapMultiByObjectType(
		Overlaps, BoxCenter, FQuat::Identity, ObjectParams, BoxShape, QueryParams);

	return bHasOverlap && Overlaps.Num() > 0;
}

bool AEnvironmentGridScanner::IsWall(int32 X, int32 Y) const
{
	if (!IsValidCell(X, Y))
	{
		return true;
	}
	return Grid[X + Y * GridWidth] != 0;
}

bool AEnvironmentGridScanner::IsPassable(int32 X, int32 Y) const
{
	return !IsWall(X, Y);
}

FVector AEnvironmentGridScanner::GetCellWorldCenter(int32 X, int32 Y) const
{
	const FVector Origin = GetActorLocation();
	const float OffsetX = (GridWidth - 1) * 0.5f * CellSize;
	const float OffsetY = (GridHeight - 1) * 0.5f * CellSize;

	const float WorldX = Origin.X + X * CellSize - OffsetX;
	const float WorldY = Origin.Y + Y * CellSize - OffsetY;
	const float WorldZ = Origin.Z;

	return FVector(WorldX, WorldY, WorldZ);
}

void AEnvironmentGridScanner::WorldToGrid(const FVector& WorldPos, int32& OutX, int32& OutY) const
{
	const FVector Origin = GetActorLocation();
	const float OffsetX = (GridWidth - 1) * 0.5f * CellSize;
	const float OffsetY = (GridHeight - 1) * 0.5f * CellSize;

	OutX = FMath::FloorToInt((WorldPos.X - Origin.X + OffsetX) / CellSize);
	OutY = FMath::FloorToInt((WorldPos.Y - Origin.Y + OffsetY) / CellSize);
}

bool AEnvironmentGridScanner::IsValidCell(int32 X, int32 Y) const
{
	return X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight;
}

int32 AEnvironmentGridScanner::GetCellValue(int32 X, int32 Y) const
{
	return IsValidCell(X, Y) ? Grid[X + Y * GridWidth] : 1;
}
