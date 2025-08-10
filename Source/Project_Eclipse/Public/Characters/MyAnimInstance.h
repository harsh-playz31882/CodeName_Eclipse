#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterTypes.h"
#include "MyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_ECLIPSE_API UMyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly)
	class AMyCharacter* MyCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	class UCharacterMovementComponent* MovementComponent;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool IsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Movement | Character State")
	ECharacterState CharacterState;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool IsMoving;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool IsJumping;
	
};
