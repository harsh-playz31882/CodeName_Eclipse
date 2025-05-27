#include "HUD/MyHUD.h"
#include "HUD/PlayerHealthBar.h"
#include "Blueprint/UserWidget.h"
#include "InputActionValue.h"

AMyHUD::AMyHUD()
{
    UE_LOG(LogTemp, Warning, TEXT("MyHUD Constructor Called"));
}

void AMyHUD::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("MyHUD BeginPlay Called"));

    if (PlayerHealthBarClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerHealthBarClass is valid"));
        PlayerHealthBarWidget = CreateWidget<UPlayerHealthBar>(GetWorld(), PlayerHealthBarClass);
        if (PlayerHealthBarWidget)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerHealthBarWidget created successfully"));
            PlayerHealthBarWidget->AddToViewport();
            PlayerHealthBarWidget->SetHealthPercent(1.f);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create PlayerHealthBarWidget"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerHealthBarClass is null"));
    }
}

void AMyHUD::DrawHUD()
{
    Super::DrawHUD();
}

void AMyHUD::UpdateHealthBar(float Percent)
{
    if (PlayerHealthBarWidget)
    {
        PlayerHealthBarWidget->SetHealthPercent(Percent);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHealthBar called but PlayerHealthBarWidget is null"));
    }
} 