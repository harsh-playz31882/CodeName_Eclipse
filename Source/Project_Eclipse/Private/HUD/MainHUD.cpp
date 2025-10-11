// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MainHUD.h"
#include "HUD/Character_Overlay.h"
#include "Enemy/Enemy.h"
#include "Components/AttributeComponent.h"


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

	// Initialize targeted enemy as null
	TargetedEnemy = nullptr;
}

void AMainHUD::SetTargetedEnemy(AEnemy* Enemy)
{
	TargetedEnemy = Enemy;
	
	// Update the enemy health bar if we have a character overlay
	if (Character_Overlay && Enemy)
	{
		// Get the enemy's health percentage and update the UI
		if (Enemy->GetAttributes())
		{
			float HealthPercent = Enemy->GetAttributes()->GetHealthPercent();
			Character_Overlay->SetEnemyHealthBarPercent(HealthPercent);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SetTargetedEnemy: Enemy has no Attributes component!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetTargetedEnemy: Character_Overlay is null or Enemy is null!"));
	}
}

void AMainHUD::ClearTargetedEnemy()
{
	TargetedEnemy = nullptr;
	
	// Hide the enemy health bar by setting it to 0
	if (Character_Overlay)
	{
		Character_Overlay->SetEnemyHealthBarPercent(0.f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ClearTargetedEnemy: Character_Overlay is null!"));
	}
}
