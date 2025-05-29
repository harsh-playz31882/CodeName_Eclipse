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

		// Debug logging
		UE_LOG(LogTemp, Warning, TEXT("Ground Speed: %.2f"), GroundSpeed);
	}
}
	