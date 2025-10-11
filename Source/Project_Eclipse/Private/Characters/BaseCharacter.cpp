#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Weapons/Weapon.h"
#include "Components/AttributeComponent.h"
#include "Engine/Engine.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	
	// Set default weapon class (can be overridden in Blueprint)
	static ConstructorHelpers::FClassFinder<AWeapon> WeaponClassFinder(TEXT("/Game/Blueprints/Weapon/BP_Weapon"));
	if (WeaponClassFinder.Succeeded())
	{
		WeaponClass = WeaponClassFinder.Class;
	}
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Spawn and equip weapon
	if (WeaponClass)
	{
		EquippedWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass);
		if (EquippedWeapon)
		{
			// Attach weapon to the character's weapon socket
			if (GetMesh()->DoesSocketExist(FName("WeaponSocket")))
			{
				EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponSocket"));
				UE_LOG(LogTemp, Warning, TEXT("BaseCharacter weapon equipped to WeaponSocket"));
			}
			else
			{
				// Fallback: attach to root component
				EquippedWeapon->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
				UE_LOG(LogTemp, Warning, TEXT("BaseCharacter weapon equipped to root (no WeaponSocket found)"));
			}
			
			// Set the weapon's owner to this character
			EquippedWeapon->SetOwner(this);
			EquippedWeapon->SetInstigator(this);
			
			// Initially disable weapon collision
			DisableWeaponCollision();
			UE_LOG(LogTemp, Warning, TEXT("BaseCharacter: Weapon collision disabled on BeginPlay"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn BaseCharacter weapon"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BaseCharacter WeaponClass is not set"));
	}
}

void ABaseCharacter::Attack()
{
}

void ABaseCharacter::PlayAttackMontage()
{
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	//angle between Forward and ToHit vectors
	// Forward*ToHit = |Forward|*|ToHit|*cos(theta), |Forward|=1, |ToHit|=1

	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	double Theta = FMath::Acos(CosTheta);



	Theta = FMath::RadiansToDegrees(Theta);

	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	FName Section("Dead");

	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("FromFront");
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName("FromRight");
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName("FromLeft");
	}

	PlayHitReactMontage(Section);
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	if (!GetMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayHitReactMontage: Mesh is null"));
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayHitReactMontage: AnimInstance is null"));
		return;
	}

	if (!HitReactMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayHitReactMontage: HitReactMontage is null"));
		return;
	}

	// Stop any currently playing montages
	AnimInstance->StopAllMontages(0.1f);
	
	// Play the hit react montage
	AnimInstance->Montage_Play(HitReactMontage, 1.0f);
	AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	
	// Set up montage end delegate
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ABaseCharacter::OnHitReactMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, HitReactMontage);

	UE_LOG(LogTemp, Warning, TEXT("PlayHitReactMontage: Successfully played montage section %s"), *SectionName.ToString());
}

void ABaseCharacter::OnHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;
	
	// Reset any state or flags after hit reaction is complete
	if (Montage == HitReactMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnHitReactMontageEnded: Hit reaction montage ended"));
	}
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::GetHit(const FVector& ImpactPoint)
{
	if (!GetMesh() || !GetMesh()->GetAnimInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetHit: Mesh or AnimInstance is null"));
		return;
	}

	if (Attributes && Attributes->IsAlive())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetHit: Playing hit reaction"));
		DirectionalHitReact(ImpactPoint);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetHit: Character is dead or has no attributes"));
	}
}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		UE_LOG(LogTemp, Warning, TEXT("BaseCharacter SetWeaponCollisionEnabled: %s"), 
			CollisionEnabled == ECollisionEnabled::QueryOnly ? TEXT("Enabled") : TEXT("Disabled"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BaseCharacter SetWeaponCollisionEnabled: No equipped weapon or weapon box"));
	}
}

void ABaseCharacter::EnableWeaponCollision()
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ClearWeaponHitActors();
		UE_LOG(LogTemp, Warning, TEXT("BaseCharacter EnableWeaponCollision: Weapon collision enabled"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BaseCharacter EnableWeaponCollision: No equipped weapon or weapon box"));
	}
}

void ABaseCharacter::DisableWeaponCollision()
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		UE_LOG(LogTemp, Warning, TEXT("BaseCharacter DisableWeaponCollision: Weapon collision disabled"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BaseCharacter DisableWeaponCollision: No equipped weapon or weapon box"));
	}
}

void ABaseCharacter::ClearWeaponHitActors()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->ClearHitActors();
	}
	HitActors.Empty();
}


