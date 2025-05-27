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
		MovementComponent->Velocity;
		GroundSpeed = UKismetMathLibrary::VSizeXY(MovementComponent->Velocity);
		IsFalling = MovementComponent->IsFalling();
		CharacterState = MyCharacter->GetCharacterState();

		if (MovementComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("Movement component is getting set!"));
			FVector Velocity = MovementComponent->Velocity;

			UE_LOG(LogTemp, Warning, TEXT("Velocity: X=%.2f, Y=%.2f, Z=%.2f"), Velocity.X, Velocity.Y, Velocity.Z);
			float Speed = Velocity.Size();

			UE_LOG(LogTemp, Warning, TEXT("Velocity: X=%.2f, Y=%.2f, Z=%.2f | Speed: %.2f"),
				Velocity.X, Velocity.Y, Velocity.Z, Speed);

		    GroundSpeed = MovementComponent->Velocity.Size(); 

			UE_LOG(LogTemp, Warning, TEXT("Ground Speed: %.2f"), GroundSpeed);


		}


	}

	

}
	