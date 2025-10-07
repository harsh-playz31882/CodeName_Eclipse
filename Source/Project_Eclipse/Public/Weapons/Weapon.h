#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Characters/MyCharacter.h"
#include "Components/BoxComponent.h"
#include "Weapon.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class USceneComponent;

UCLASS(Blueprintable)
class PROJECT_ECLIPSE_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* SwordMesh;

	UPROPERTY(VisibleAnywhere, Category=WeaponProperties)
	UBoxComponent* WeaponBox;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceStart;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceEnd;

	UPROPERTY(EditAnywhere, Category = WeaponProperties)
	float Damage = 20.f;

	// Track which actors have been hit to prevent multiple hits
	UPROPERTY()
	TArray<AActor*> HitActors;

    // Helper function to perform box trace (debug drawing removed)
    bool BoxTrace(FHitResult& OutHit);

	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);

public:
	FORCEINLINE UBoxComponent* GetWeaponBox() const { return WeaponBox; }
	FORCEINLINE UStaticMeshComponent* GetSwordMesh() const { return SwordMesh; }
	
	// Clear the hit actors list (call this when starting a new attack)
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ClearHitActors();

private:
    // Recursively gather all actors attached to the given root actor
    static void GatherAttachedActorsRecursive(AActor* RootActor, TSet<AActor*>& OutAttached);
};
