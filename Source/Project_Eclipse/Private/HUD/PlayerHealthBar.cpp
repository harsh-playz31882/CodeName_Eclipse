#include "HUD/PlayerHealthBar.h"
#include "Components/ProgressBar.h"

void UPlayerHealthBar::SetHealthPercent(float Percent)
{
    if (HealthBar)
    {
        HealthBar->SetPercent(Percent);
    }
} 