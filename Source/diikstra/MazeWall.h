// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MazeWall.generated.h"

class UStaticMeshComponent;

UCLASS()
class DIIKSTRA_API AMazeWall : public AActor
{
	GENERATED_BODY()

public:
	AMazeWall();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

protected:
	virtual void BeginPlay() override;
};
