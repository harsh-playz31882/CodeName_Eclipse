#include "Animation/ANS_EnableWeaponCollision.h"
#include "Characters/MyCharacter.h"
#include "Characters/BaseCharacter.h"
#include "Enemy/Enemy.h"
#include "Weapons/Weapon.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"

void UANS_EnableWeaponCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner()))
    {
        if (AMyCharacter* MyCharacter = Cast<AMyCharacter>(Character))
        {
            // For player character
            MyCharacter->EnableWeaponCollision();
            
            // Clear hit actors when starting weapon collision
            MyCharacter->ClearWeaponHitActors();
            
            UE_LOG(LogTemp, Warning, TEXT("EnableWeaponCollision: Player weapon collision enabled"));
        }
        else if (AEnemy* Enemy = Cast<AEnemy>(Character))
        {
            // For enemy
            Enemy->SetWeaponCollisionEnabled(ECollisionEnabled::QueryOnly);
            UE_LOG(LogTemp, Warning, TEXT("EnableWeaponCollision: Enemy weapon collision enabled"));
        }
    }
}

void UANS_EnableWeaponCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner()))
    {
        if (AMyCharacter* MyCharacter = Cast<AMyCharacter>(Character))
        {
            // For player character
            MyCharacter->DisableWeaponCollision();
            UE_LOG(LogTemp, Warning, TEXT("EnableWeaponCollision: Player weapon collision disabled"));
        }
        else if (AEnemy* Enemy = Cast<AEnemy>(Character))
        {
            // For enemy
            Enemy->SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
            UE_LOG(LogTemp, Warning, TEXT("EnableWeaponCollision: Enemy weapon collision disabled"));
        }
    }
} 