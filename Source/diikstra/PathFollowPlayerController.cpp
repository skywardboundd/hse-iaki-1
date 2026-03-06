// Copyright Epic Games, Inc. All Rights Reserved.

#include "PathFollowPlayerController.h"
#include "EnvironmentGridScanner.h"
#include "diikstraCharacter.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"

APathFollowPlayerController::APathFollowPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APathFollowPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!GridScanner && GetWorld())
	{
		for (TActorIterator<AEnvironmentGridScanner> It(GetWorld()); It; ++It)
		{
			GridScanner = *It;
			break;
		}
	}
}

void APathFollowPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindKey(PathFollowKey, EInputEvent::IE_Pressed, this, &APathFollowPlayerController::TogglePathFollow);
		InputComponent->BindKey(TeleportToStartKey, EInputEvent::IE_Pressed, this, &APathFollowPlayerController::TeleportToStart);
	}
}

void APathFollowPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bPathFollowEnabled)
	{
		UpdatePathFollow(DeltaTime);
	}
}

void APathFollowPlayerController::TogglePathFollow()
{
	bPathFollowEnabled = !bPathFollowEnabled;
	if (bPathFollowEnabled)
	{
		StartPathFollow();
	}
	else
	{
		StopPathFollow();
	}
}

void APathFollowPlayerController::StartPathFollow()
{
	if (!GridScanner)
	{
		UE_LOG(LogTemp, Warning, TEXT("PathFollow: GridScanner not found"));
		return;
	}

	if (GetPawn() && GridScanner->StartCell.X >= 0 && GridScanner->StartCell.Y >= 0)
	{
		const FVector StartPos = GridScanner->GetCellWorldCenter(GridScanner->StartCell.X, GridScanner->StartCell.Y);
		GetPawn()->SetActorLocation(StartPos);
	}

	GridScanner->FindPath();

	if (GridScanner->PathResult.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("PathFollow: No path or path too short"));
		bPathFollowEnabled = false;
		return;
	}

	PathIndex = 0;
	bPathFollowEnabled = true;

	const FIntPoint& Current = GridScanner->PathResult[PathIndex];
	const FIntPoint& Next = GridScanner->PathResult[PathIndex + 1];
	const int32 dX = Next.X - Current.X;
	const int32 dY = Next.Y - Current.Y;
	TargetYaw = GetYawForDirection(dX, dY);
	SetControlRotation(FRotator(0.0f, TargetYaw, 0.0f));

	UE_LOG(LogTemp, Log, TEXT("PathFollow: Started, %d cells"), GridScanner->PathResult.Num());
}

void APathFollowPlayerController::StopPathFollow()
{
	bPathFollowEnabled = false;

	if (AdiikstraCharacter* Char = Cast<AdiikstraCharacter>(GetPawn()))
	{
		Char->DoMove(0.0f, 0.0f);
	}
}

void APathFollowPlayerController::TeleportToStart()
{
	if (!GridScanner)
	{
		return;
	}

	GridScanner->StartPathfindingAnimation();

	if (!GetPawn())
	{
		return;
	}

	if (GridScanner->StartCell.X < 0 || GridScanner->StartCell.Y < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PathFollow: Start cell not found"));
		return;
	}

	const FVector StartPos = GridScanner->GetCellWorldCenter(GridScanner->StartCell.X, GridScanner->StartCell.Y);
	GetPawn()->SetActorLocation(StartPos);

	StopPathFollow();
	PathIndex = 0;

	UE_LOG(LogTemp, Log, TEXT("PathFollow: Teleported to start, path search started"));
}

void APathFollowPlayerController::UpdatePathFollow(float DeltaTime)
{
	if (!GridScanner || !GetPawn())
	{
		bPathFollowEnabled = false;
		return;
	}

	const TArray<FIntPoint>& Path = GridScanner->PathResult;
	if (PathIndex >= Path.Num() - 1)
	{
		if (AdiikstraCharacter* Char = Cast<AdiikstraCharacter>(GetPawn()))
		{
			Char->DoMove(0.0f, 0.0f);
		}
		bPathFollowEnabled = false;
		UE_LOG(LogTemp, Log, TEXT("PathFollow: Reached finish"));
		return;
	}

	const FIntPoint& TargetCell = Path[PathIndex + 1];
	const FVector TargetWorld = GridScanner->GetCellWorldCenter(TargetCell.X, TargetCell.Y);
	const FVector PawnPos = GetPawn()->GetActorLocation();
	const FVector ToTarget = TargetWorld - PawnPos;
	const float Dist2D = FVector(ToTarget.X, ToTarget.Y, 0).Size();

	const int32 dX = TargetCell.X - Path[PathIndex].X;
	const int32 dY = TargetCell.Y - Path[PathIndex].Y;
	TargetYaw = GetYawForDirection(dX, dY);

	float CurrentYaw = GetControlRotation().Yaw;
	float DeltaYaw = TargetYaw - CurrentYaw;
	while (DeltaYaw > 180.0f) DeltaYaw -= 360.0f;
	while (DeltaYaw < -180.0f) DeltaYaw += 360.0f;

	const float MaxTurn = TurnSpeed * DeltaTime;
	if (FMath::Abs(DeltaYaw) <= MaxTurn)
	{
		SetControlRotation(FRotator(0.0f, TargetYaw, 0.0f));
	}
	else
	{
		CurrentYaw += FMath::Sign(DeltaYaw) * MaxTurn;
		SetControlRotation(FRotator(0.0f, CurrentYaw, 0.0f));
	}

	if (AdiikstraCharacter* Char = Cast<AdiikstraCharacter>(GetPawn()))
	{
		Char->DoMove(0.0f, 1.0f);
	}

	if (Dist2D <= CellReachRadius)
	{
		PathIndex++;
	}
}

float APathFollowPlayerController::GetYawForDirection(int32 dX, int32 dY) const
{
	if (dX == 1 && dY == 0) return 0.0f;
	if (dX == -1 && dY == 0) return 180.0f;
	if (dX == 0 && dY == 1) return 90.0f;
	if (dX == 0 && dY == -1) return -90.0f;
	return 0.0f;
}
