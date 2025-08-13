#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.h"	
#include "CharacterTypes.h"
#include "InputActionValue.h"
#include "MyCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class UBoxComponent;
class UCharacter_Overlay;
class UInputMappingContext;
class UInputAction;

UCLASS()
class PROJECT_ECLIPSE_API AMyCharacter : public ABaseCharacter 
{
	GENERATED_BODY()

public:
	AMyCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void PlayAttackMontage() override;
	virtual void Attack() override;
	virtual void Jump() override;

	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }

	void EnableKickCollision();
	void DisableKickCollision();


	void EnableWeaponCollision();
	void DisableWeaponCollision();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;


	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ControlAnimationRootMotion();

	void StopMontages();

	void GetCharacterMovements();
	
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	
	void AttackEnd() override;

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void GetHit(const FVector& ImpactPoint);

	virtual void OnHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted) override;


private:
	void InitializeCharacterOverlay();

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	// Add two box components for both legs
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* KickBoxLeft;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* KickBoxRight;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY()
	UCharacter_Overlay* Character_Overlay;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	int32 AttackCount = 0;

	UFUNCTION()
	void OnKickBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

