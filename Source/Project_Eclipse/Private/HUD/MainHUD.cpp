// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MainHUD.h"
#include "HUD/Character_Overlay.h"


void AMainHUD::BeginPlay()
{
	Super::BeginPlay();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController && Character_OverlayClass)
		{
			Character_Overlay = CreateWidget<UCharacter_Overlay>(PlayerController, Character_OverlayClass);
			Character_Overlay->AddToViewport();
		}
	}

}
