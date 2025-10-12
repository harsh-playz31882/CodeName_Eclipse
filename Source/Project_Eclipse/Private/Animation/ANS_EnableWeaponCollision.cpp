#include "Animation/ANS_EnableWeaponCollision.h"
#include "Characters/MyCharacter.h"
#include "Characters/BaseCharacter.h"
#include "Enemy/Enemy.h"
#include "Weapons/Weapon.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"

void UANS_EnableWeaponCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Magenta, TEXT("ANIMATION NOTIFY: EnableWeaponCollision BEGIN"));
    
    if (ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner()))
    {
        if (AMyCharacter* MyCharacter = Cast<AMyCharacter>(Character))
        {
            // For player character
            MyCharacter->EnableWeaponCollision();
            
            // Clear hit actors when starting weapon collision
            MyCharacter->ClearWeaponHitActors();
            
            UE_LOG(LogTemp, Warning, TEXT("EnableWeaponCollision: Player weapon collision enabled"));
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("PLAYER WEAPON COLLISION ENABLED"));
        }
        else if (AEnemy* Enemy = Cast<AEnemy>(Character))
        {
            // For enemy
            Enemy->SetWeaponCollisionEnabled(ECollisionEnabled::QueryOnly);
            UE_LOG(LogTemp, Warning, TEXT("EnableWeaponCollision: Enemy weapon collision enabled"));
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("ENEMY WEAPON COLLISION ENABLED"));
        }
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("ANIMATION NOTIFY: No BaseCharacter found"));
    }
}

void UANS_EnableWeaponCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Magenta, TEXT("ANIMATION NOTIFY: EnableWeaponCollision END"));
    
    if (ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner()))
    {
        if (AMyCharacter* MyCharacter = Cast<AMyCharacter>(Character))
        {
            // For player character
            MyCharacter->DisableWeaponCollision();
            UE_LOG(LogTemp, Warning, TEXT("EnableWeaponCollision: Player weapon collision disabled"));
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("PLAYER WEAPON COLLISION DISABLED"));
        }
        else if (AEnemy* Enemy = Cast<AEnemy>(Character))
        {
            // For enemy
            Enemy->SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
            UE_LOG(LogTemp, Warning, TEXT("EnableWeaponCollision: Enemy weapon collision disabled"));
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("ENEMY WEAPON COLLISION DISABLED"));
        }
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("ANIMATION NOTIFY END: No BaseCharacter found"));
    }
} 