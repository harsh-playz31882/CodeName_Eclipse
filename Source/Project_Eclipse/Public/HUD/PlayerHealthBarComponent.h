#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "PlayerHealthBarComponent.generated.h"

UCLASS()
class PROJECT_ECLIPSE_API UPlayerHealthBarComponent : public UWidgetComponent
{
    GENERATED_BODY()

public:
    UPlayerHealthBarComponent();

    void SetHealthPercent(float Percent);

private:
    UPROPERTY()
    class UPlayerHealthBar* PlayerHealthBarWidget;
}; 