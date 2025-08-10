#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Weapons/Weapon.h"
#include "Components/AttributeComponent.h"
#include "Engine/Engine.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
	WeaponBox->SetupAttachment(GetRootComponent());
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
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
	if (WeaponBox)
	{
		WeaponBox->SetCollisionEnabled(CollisionEnabled);
	}
	
}


