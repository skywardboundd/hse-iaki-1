// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameZone.generated.h"

class UBoxComponent;

UENUM(BlueprintType)
enum class EGameZoneType : uint8
{
	Start,
	Finish
};

UCLASS()
class DIIKSTRA_API AGameZone : public AActor
{
	GENERATED_BODY()

public:
	AGameZone();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	EGameZoneType ZoneType = EGameZoneType::Start;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> VerticalLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone", meta = (ClampMin = 50, ClampMax = 2000))
	float LineHeight = 500.0f;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
};
