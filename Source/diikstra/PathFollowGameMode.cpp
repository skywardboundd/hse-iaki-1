// Copyright Epic Games, Inc. All Rights Reserved.

#include "PathFollowGameMode.h"
#include "PathFollowPlayerController.h"
#include "PathFollowHUD.h"
#include "diikstraCharacter.h"

APathFollowGameMode::APathFollowGameMode()
{
	PlayerControllerClass = APathFollowPlayerController::StaticClass();
	HUDClass = APathFollowHUD::StaticClass();
}
