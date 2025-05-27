#include "HUD/PlayerHealthBarComponent.h"
#include "HUD/PlayerHealthBar.h"

UPlayerHealthBarComponent::UPlayerHealthBarComponent()
{
    SetWidgetSpace(EWidgetSpace::Screen);
    SetDrawSize(FVector2D(200.f, 20.f));
    SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    bHiddenInGame = false;
    SetVisibility(true);
}

void UPlayerHealthBarComponent::SetHealthPercent(float Percent)
{
    if (PlayerHealthBarWidget == nullptr)
    {
        PlayerHealthBarWidget = Cast<UPlayerHealthBar>(GetUserWidgetObject());
    }
    
    if (PlayerHealthBarWidget)
    {
        PlayerHealthBarWidget->SetHealthPercent(Percent);
    }
} 