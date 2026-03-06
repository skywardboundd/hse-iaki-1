// Copyright Epic Games, Inc. All Rights Reserved.

#include "GridPathfinder.h"
#include "Containers/Set.h"
#include "Math/NumericLimits.h"
#include "Templates/Function.h"

namespace
{
	const int32 Dx[] = { 0, 1, 0, -1 };
	const int32 Dy[] = { -1, 0, 1, 0 };

	float GetWeight(const TArray<float>& Weights, int32 X, int32 Y, int32 InGridWidth)
	{
		const int32 Idx = X + Y * InGridWidth;
		return Weights.IsValidIndex(Idx) ? Weights[Idx] : 0.0f;
	}

	void RebuildPath(
		FGridPathfinder::FResult& Result,
		const TArray<FIntPoint>& Prev,
		int32 InGridWidth,
		FIntPoint Finish)
	{
		FIntPoint Cur = Finish;
		while (true)
		{
			Result.Path.Insert(Cur, 0);
			const FIntPoint P = Prev[Cur.X + Cur.Y * InGridWidth];
			if (P.X < 0 || P.Y < 0)
			{
				break;
			}
			Cur = P;
		}
	}

	FGridPathfinder::FResult FindPathWithHeuristic(
		const TArray<uint8>& Grid,
		const TArray<float>& Weights,
		int32 InGridWidth,
		int32 InGridHeight,
		FIntPoint Start,
		FIntPoint Finish,
		TFunctionRef<float(const FIntPoint& Vertex)> Heuristic)
	{
		FGridPathfinder::FResult Result;

		const int32 NumCells = InGridWidth * InGridHeight;
		const float Inf = TNumericLimits<float>::Max();

		TArray<float> GScore;
		GScore.Init(Inf, NumCells);

		TArray<FIntPoint> Prev;
		Prev.Init(FIntPoint(-1, -1), NumCells);

		using FQueueElement = TPair<float, FIntPoint>;
		TSet<FIntPoint> ClosedSet;
		TSet<FQueueElement> OpenSet;

		const int32 StartIdx = Start.X + Start.Y * InGridWidth;
		GScore[StartIdx] = 0.0f;
		OpenSet.Add(FQueueElement(Heuristic(Start), Start));

		while (OpenSet.Num() > 0)
		{
			FQueueElement Best(Inf, FIntPoint(-1, -1));
			for (const FQueueElement& E : OpenSet)
			{
				if (E.Key < Best.Key)
				{
					Best = E;
				}
			}

			const FIntPoint Cur = Best.Value;
			OpenSet.Remove(Best);

			if (ClosedSet.Contains(Cur))
			{
				continue;
			}
			ClosedSet.Add(Cur);
			Result.ExploredOrder.Add(Cur);

			if (Cur == Finish)
			{
				Result.bSuccess = true;
				Result.OpenSetPerStep.Add(TSet<FIntPoint>());
				break;
			}

			const int32 CurIdx = Cur.X + Cur.Y * InGridWidth;
			const float CurG = GScore[CurIdx];

			for (int32 I = 0; I < 4; ++I)
			{
				const int32 Nx = Cur.X + Dx[I];
				const int32 Ny = Cur.Y + Dy[I];

				if (Nx < 0 || Nx >= InGridWidth || Ny < 0 || Ny >= InGridHeight)
				{
					continue;
				}

				const FIntPoint NextPoint(Nx, Ny);
				const int32 NIdx = Nx + Ny * InGridWidth;
				if (Grid[NIdx] != 0 || ClosedSet.Contains(NextPoint))
				{
					continue;
				}

				const float EdgeCost = GetWeight(Weights, Nx, Ny, InGridWidth);
				const float TentativeG = CurG + EdgeCost;
				if (TentativeG < GScore[NIdx])
				{
					GScore[NIdx] = TentativeG;
					Prev[NIdx] = Cur;
					OpenSet.Add(FQueueElement(TentativeG + Heuristic(NextPoint), NextPoint));
				}
			}

			TSet<FIntPoint> OpenSetCells;
			for (const FQueueElement& E : OpenSet)
			{
				OpenSetCells.Add(E.Value);
			}
			Result.OpenSetPerStep.Add(OpenSetCells);
		}

		if (!Result.bSuccess)
		{
			return Result;
		}

		RebuildPath(Result, Prev, InGridWidth, Finish);
		return Result;
	}
}

FGridPathfinder::FResult FGridPathfinder::FindPath(
	const TArray<uint8>& Grid,
	const TArray<float>& Weights,
	int32 InGridWidth,
	int32 InGridHeight,
	FIntPoint Start,
	FIntPoint Finish)
{
	return FindPathDijkstra(Grid, Weights, InGridWidth, InGridHeight, Start, Finish);
}

FGridPathfinder::FResult FGridPathfinder::FindPathDijkstra(
	const TArray<uint8>& Grid,
	const TArray<float>& Weights,
	int32 InGridWidth,
	int32 InGridHeight,
	FIntPoint Start,
	FIntPoint Finish)
{
	return FindPathWithHeuristic(
		Grid,
		Weights,
		InGridWidth,
		InGridHeight,
		Start,
		Finish,
		[](const FIntPoint&)
		{
			return 0.0f;
		});
}

FGridPathfinder::FResult FGridPathfinder::FindPathAStar(
	const TArray<uint8>& Grid,
	const TArray<float>& Weights,
	int32 InGridWidth,
	int32 InGridHeight,
	FIntPoint Start,
	FIntPoint Finish)
{
	return FindPathWithHeuristic(
		Grid,
		Weights,
		InGridWidth,
		InGridHeight,
		Start,
		Finish,
		[Finish](const FIntPoint& Vertex)
		{
			return static_cast<float>(FMath::Abs(Vertex.X - Finish.X) + FMath::Abs(Vertex.Y - Finish.Y));
		});
}
