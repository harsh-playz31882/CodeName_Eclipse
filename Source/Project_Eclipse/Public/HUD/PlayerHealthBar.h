#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD/MyHUD.h"
#include "PlayerHealthBar.generated.h"

UCLASS()
class PROJECT_ECLIPSE_API UPlayerHealthBar : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    class UProgressBar* HealthBar;

    void SetHealthPercent(float Percent);
};
