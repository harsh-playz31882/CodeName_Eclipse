#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;

UCLASS()
class PROJECT_ECLIPSE_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:

	ABaseCharacter();
	virtual void Tick(float DeltaTime);
	virtual void GetHit(const FVector& ImpactPoint);

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);


protected:
	
	virtual void BeginPlay() override;

	virtual void Attack();
	virtual void PlayAttackMontage();
	virtual void PlayHitReactMontage(const FName& SectionName);

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* WeaponBox;

	UFUNCTION()
	UBoxComponent* GetWeaponBox() const { return WeaponBox; }

	//Animation Montages
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* HitReactMontage;

	void DirectionalHitReact(const FVector& ImpactPoint);

	UFUNCTION()
	virtual void OnHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;


	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = VisualEffects)
	UParticleSystem* HitParticles;






};

