// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Set.h"

/** Поиск пути на сетке: Dijkstra и A*. */
struct DIIKSTRA_API FGridPathfinder
{
	/** Результат поиска */
	struct FResult
	{
		/** Путь от старта до финиша */
		TArray<FIntPoint> Path;

		/** Порядок обхода */
		TArray<FIntPoint> ExploredOrder;

		/** OpenSet (кандидаты) на каждом шаге — для визуализации */
		TArray<TSet<FIntPoint>> OpenSetPerStep;

		bool bSuccess = false;
	};

	/** Поиск пути (по умолчанию Дейкстра) */
	static FResult FindPath(
		const TArray<uint8>& Grid,
		const TArray<float>& Weights,
		int32 InGridWidth,
		int32 InGridHeight,
		FIntPoint Start,
		FIntPoint Finish);

	/** Дейкстра */
	static FResult FindPathDijkstra(
		const TArray<uint8>& Grid,
		const TArray<float>& Weights,
		int32 InGridWidth,
		int32 InGridHeight,
		FIntPoint Start,
		FIntPoint Finish);

	/** A* (манхэттен) */
	static FResult FindPathAStar(
		const TArray<uint8>& Grid,
		const TArray<float>& Weights,
		int32 InGridWidth,
		int32 InGridHeight,
		FIntPoint Start,
		FIntPoint Finish);
};
