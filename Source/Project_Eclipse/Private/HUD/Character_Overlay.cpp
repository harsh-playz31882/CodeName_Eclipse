// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/Character_Overlay.h"
#include "Components/ProgressBar.h"


void UCharacter_Overlay::SetHealthBarPercent(float Percent)
{
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(Percent);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetHealthBarPercent: HealthProgressBar is null! Make sure your Blueprint widget has a ProgressBar with variable name 'HealthProgressBar'"));
	}
}

void UCharacter_Overlay::SetStaminaBarPercent(float Percent)
{
	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(Percent);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetStaminaBarPercent: StaminaProgressBar is null! Make sure your Blueprint widget has a ProgressBar with variable name 'StaminaProgressBar'"));
	}
}

void UCharacter_Overlay::SetEnemyHealthBarPercent(float Percent)
{
	if (EnemyHealthProgressBar)
	{
		EnemyHealthProgressBar->SetPercent(Percent);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetEnemyHealthBarPercent: EnemyHealthProgressBar is null! Make sure your Blueprint widget has a ProgressBar with variable name 'EnemyHealthProgressBar'"));
	}
}

void UCharacter_Overlay::SetEnemyStaminaBarPercent(float Percent)
{
	if (EnemyStaminaProgressBar)
	{
		EnemyStaminaProgressBar->SetPercent(Percent);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetEnemyStaminaBarPercent: EnemyStaminaProgressBar is null! Make sure your Blueprint widget has a ProgressBar with variable name 'EnemyStaminaProgressBar'"));
	}
}

