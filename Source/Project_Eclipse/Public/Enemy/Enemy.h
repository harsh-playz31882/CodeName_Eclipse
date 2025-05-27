// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/HitInterface.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UAnimMontage;
class UAttributeComponent;
class UHealthBarComponent;
class UAIPerceptionComponent;
class AWeapon;

UCLASS()
class PROJECT_ECLIPSE_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose;

	EEnemyState EnemyState = EEnemyState::EES_Patrolling;	

protected:
	virtual void BeginPlay() override;

	void Die();


	bool InTargetRange(AActor* Target, double Radius);

	void MoveToTarget(AActor* Target);
	
	virtual void Attack() override;
	virtual void PlayAttackMontage() override;

	AActor* ChoosePatrolTarget();

	void PlayHitReactMontage(const FName& SectionName);

public:	
	virtual void Tick(float DeltaTime) override;

	void CheckPatroTarget();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AWeapon> WeaponClass;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetHit(const FVector& ImpactPoint) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// Add OnPerceptionUpdated function declaration
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
	// Add AttackRange property
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 150.f;

private:
	//Components
	UPROPERTY(VisibleAnywhere)
	UAIPerceptionComponent* AIPerception;

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget1;

	/*
	 Animation Montages
	*/

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DeathMontage;

	/*
	* Navigation
	*/

	UPROPERTY()
	class AAIController* EnemyController;

	//current patrol target
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;

	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();

	UPROPERTY()
	AWeapon* EquippedWeapon;
};
