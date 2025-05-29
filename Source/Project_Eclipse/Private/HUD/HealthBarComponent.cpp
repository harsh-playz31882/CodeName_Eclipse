// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/HealthBarComponent.h"
#include "HUD/HealthBar1.h"
#include "Components/ProgressBar.h"

void UHealthBarComponent::SetHealthPercent(float Percent)
{
	if (HealthBarWidget1 == nullptr)
	{
		HealthBarWidget1 = Cast<UHealthBar1>(GetUserWidgetObject());
	}

	if (HealthBarWidget1 && HealthBarWidget1->HealthBar1)
	{
		HealthBarWidget1->HealthBar1->SetPercent(Percent);
	}
}