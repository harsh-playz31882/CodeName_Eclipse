#include "Enemy/EnemyAnimInstance.h"
#include "Enemy/Enemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UEnemyAnimInstance::UEnemyAnimInstance()
{
	bIsDead = false;
	DeathPose = EDeathPose::EDP_Alive;
	GroundSpeed = 0.f;
	IsFalling = false;
}

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Enemy = Cast<AEnemy>(TryGetPawnOwner());
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (Enemy == nullptr)
	{
		Enemy = Cast<AEnemy>(TryGetPawnOwner());
		if (Enemy == nullptr) return;
	}

	// Update death state
	bIsDead = Enemy->bIsDead;
	DeathPose = Enemy->DeathPose;

	// If enemy is dead, don't update other animation properties
	if (bIsDead) return;

	// Update movement properties
	UCharacterMovementComponent* MovementComponent = Enemy->GetCharacterMovement();
	if (MovementComponent)
	{
		GroundSpeed = MovementComponent->Velocity.Size2D();
		IsFalling = MovementComponent->IsFalling();
	}
} 