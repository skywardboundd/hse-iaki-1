// Copyright Epic Games, Inc. All Rights Reserved.

#include "PathFollowHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "CanvasItem.h"

void APathFollowHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	const float X = Margin;
	const float Y = Canvas->SizeY - Margin - 30.0f;

	FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(HintsText), Font, FLinearColor::White);
	TextItem.Scale = FVector2D(TextScale, TextScale);
	Canvas->DrawItem(TextItem);
}
