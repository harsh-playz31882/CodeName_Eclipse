// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/Character_Overlay.h"
#include "Components/ProgressBar.h"


void UCharacter_Overlay::SetHealthBarPercent(float Percent)
{
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(Percent);
	}
}

void UCharacter_Overlay::SetStaminaBarPercent(float Percent)
{
	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(Percent);
	}
}
