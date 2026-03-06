// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "diikstraPlayerController.h"
#include "PathFollowPlayerController.generated.h"

class AEnvironmentGridScanner;
class AdiikstraCharacter;

UCLASS()
class DIIKSTRA_API APathFollowPlayerController : public AdiikstraPlayerController
{
	GENERATED_BODY()

public:
	APathFollowPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "PathFollow")
	void TogglePathFollow();

	UFUNCTION(BlueprintCallable, Category = "PathFollow")
	void StartPathFollow();

	UFUNCTION(BlueprintCallable, Category = "PathFollow")
	void StopPathFollow();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathFollow")
	TObjectPtr<AEnvironmentGridScanner> GridScanner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathFollow", meta = (ClampMin = 10, ClampMax = 200))
	float CellReachRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathFollow", meta = (ClampMin = 90, ClampMax = 720))
	float TurnSpeed = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathFollow")
	FKey PathFollowKey = EKeys::F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathFollow")
	FKey TeleportToStartKey = EKeys::P;

private:
	void TeleportToStart();

	void UpdatePathFollow(float DeltaTime);
	float GetYawForDirection(int32 dX, int32 dY) const;

	bool bPathFollowEnabled = false;
	int32 PathIndex = 0;
	float TargetYaw = 0.0f;
};
