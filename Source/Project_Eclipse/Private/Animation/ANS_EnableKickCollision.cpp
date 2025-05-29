#include "Animation/ANS_EnableKickCollision.h"
#include "Characters/MyCharacter.h"

void UANS_EnableKickCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (AMyCharacter* Character = Cast<AMyCharacter>(MeshComp->GetOwner()))
    {
        Character->EnableKickCollision();
    }
}

void UANS_EnableKickCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (AMyCharacter* Character = Cast<AMyCharacter>(MeshComp->GetOwner()))
    {
        Character->DisableKickCollision();
    }
} 