#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.h"	
#include "CharacterTypes.h"
#include "MyCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class UBoxComponent;

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

	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }

	void EnableKickCollision();
	void DisableKickCollision();


	void EnableWeaponCollision();
	void DisableWeaponCollision();

protected:
	virtual void BeginPlay() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	
	void AttackEnd() override;

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* KickBox;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	int32 AttackCount = 0;

	UFUNCTION()
	void OnKickBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

