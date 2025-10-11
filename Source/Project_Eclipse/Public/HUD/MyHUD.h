#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyHUD.generated.h"

UCLASS()
class PROJECT_ECLIPSE_API AMyHUD : public AHUD
{
    GENERATED_BODY()

public:
    AMyHUD();

    virtual void BeginPlay() override;
    virtual void DrawHUD() override;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UUserWidget> PlayerHealthBarClass;

    UPROPERTY()
    class UPlayerHealthBar* PlayerHealthBarWidget;

    void UpdateHealthBar(float Percent);
    

};