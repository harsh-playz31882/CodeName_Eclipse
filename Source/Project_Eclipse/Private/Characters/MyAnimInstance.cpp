// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MyAnimInstance.h"
#include "Characters/MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UMyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	MyCharacter = Cast<AMyCharacter>(TryGetPawnOwner());
	if(MyCharacter)
	{
		MovementComponent = MyCharacter->GetCharacterMovement();
	}



}

void UMyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (MovementComponent)
	{
		// Calculate ground speed using only horizontal movement
		FVector Velocity = MovementComponent->Velocity;
		GroundSpeed = UKismetMathLibrary::VSizeXY(Velocity);
		IsFalling = MovementComponent->IsFalling();
		
		if (MyCharacter)
		{
			CharacterState = MyCharacter->GetCharacterState();
		}

		// Calculate additional movement states
		IsMoving = GroundSpeed > 0.1f;
		IsJumping = IsFalling;
		
		// Enhanced debug logging
		UE_LOG(LogTemp, Warning, TEXT("Animation BP - Ground Speed: %.2f, IsFalling: %s, IsMoving: %s, Character State: %d"), 
			GroundSpeed, 
			IsFalling ? TEXT("True") : TEXT("False"),
			IsMoving ? TEXT("True") : TEXT("False"),
			(int32)CharacterState);
		
		// Log specific speed ranges for debugging transitions
		if (GroundSpeed > 0 && GroundSpeed < 10)
		{
			UE_LOG(LogTemp, Warning, TEXT("Slow movement detected: %.2f"), GroundSpeed);
		}
		else if (GroundSpeed >= 10 && GroundSpeed < 100)
		{
			UE_LOG(LogTemp, Warning, TEXT("Walking speed detected: %.2f"), GroundSpeed);
		}
		else if (GroundSpeed >= 100)
		{
			UE_LOG(LogTemp, Warning, TEXT("Running speed detected: %.2f"), GroundSpeed);
		}
		
		// Log when player is moving
		if (IsMoving)
		{
			UE_LOG(LogTemp, Warning, TEXT("Player is moving - Speed: %.2f"), GroundSpeed);
		}
		
		// Log when player is jumping
		if (IsJumping)
		{
			UE_LOG(LogTemp, Warning, TEXT("Player is jumping/falling"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MovementComponent is null in Animation Blueprint"));
	}
}
	