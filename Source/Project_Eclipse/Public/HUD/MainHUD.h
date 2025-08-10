// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MainHUD.generated.h"

/**
 * 
 */

class UCharacter_Overlay;
UCLASS()
class PROJECT_ECLIPSE_API AMainHUD : public AHUD
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "CharacterHUD")
	TSubclassOf<UCharacter_Overlay> Character_OverlayClass;

	UPROPERTY()
	UCharacter_Overlay* Character_Overlay;

public:
	FORCEINLINE UCharacter_Overlay* GetCharacterOverlay() const { return Character_Overlay; }


	
};
