// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PathFollowHUD.generated.h"

UCLASS()
class DIIKSTRA_API APathFollowHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	FString HintsText = TEXT("D - Dijkstra | A - A* | F - идти по пути | P - телепорт в старт");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD", meta = (ClampMin = 0, ClampMax = 100))
	float Margin = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD", meta = (ClampMin = 0.5, ClampMax = 3.0))
	float TextScale = 1.5f;
};
