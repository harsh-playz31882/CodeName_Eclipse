#include "Characters/MyCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/BoxComponent.h"
#include "Weapons/Weapon.h"
#include "Components/AttributeComponent.h"
#include "HUD/MyHUD.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/HitInterface.h"
#include "Perception/AIPerceptionComponent.h"




AMyCharacter::AMyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Create and setup camera components first
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->TargetArmLength = 300.f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bInheritPitch = true;
    CameraBoom->bInheritYaw = true;
    CameraBoom->bInheritRoll = false;
    CameraBoom->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));

    ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
    ViewCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    ViewCamera->bUsePawnControlRotation = false;

    // Create kick box component
    KickBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Kick Box"));
    KickBox->SetupAttachment(GetMesh(), FName("foot_l"));
    KickBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    KickBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    KickBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

    AttackMontage = CreateDefaultSubobject<UAnimMontage>(TEXT("/Game/Animations/KYRA_Animations/AM_AttackMontage2.AM_AttackMontage2"));
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Configure character movement settings
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
		GetCharacterMovement()->bUseRVOAvoidance = true;
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
		GetCharacterMovement()->JumpZVelocity = 500.f;
		GetCharacterMovement()->AirControl = 0.2f;
	}

	// Stop any playing montages
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.0f);
	}

	USkeletalMeshComponent* MeshComp = GetMesh(); // Get the skeletal mesh component

	if (MeshComp && MeshComp->GetSkeletalMeshAsset())
	{
		if (MeshComp->DoesSocketExist(FName("WeaponSocket")))
		{
			FVector SocketLocation = MeshComp->GetSocketLocation(FName("WeaponSocket"));
			UE_LOG(LogTemp, Warning, TEXT("WeaponSocket Location: %s"), *SocketLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("WeaponSocket not found on Skeletal Mesh!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Skeletal Mesh found on Character!"));
	}

	// Attach weapon box to socket after mesh is fully initialized
	if (WeaponBox && GetMesh())
	{
		WeaponBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("weapon_r"));
		// Ensure weapon collision is disabled by default
		WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	}

	// Set up kick box overlap events
	KickBox->OnComponentBeginOverlap.AddDynamic(this, &AMyCharacter::OnKickBoxOverlap);

	Tags.Add(FName("MyCharacter"));

}

void AMyCharacter::MoveForward(float Value)
{
	if (Controller && (Value != 0.f) && ActionState == EActionState::EAS_Unoccupied)
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMyCharacter::MoveRight(float Value)
{
	if (Controller && (Value != 0.f) && ActionState == EActionState::EAS_Unoccupied)
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AMyCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AMyCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AMyCharacter::Attack()
{
	if (ActionState == EActionState::EAS_Unoccupied)
	{
		// Disable movement rotation during attack
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->bOrientRotationToMovement = false;
			GetCharacterMovement()->bUseControllerDesiredRotation = true;
		}

		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}


void AMyCharacter::PlayAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		// Ensure weapon collision is disabled before starting the attack
		DisableWeaponCollision();
		DisableKickCollision();
		
		// Set montage blend settings for smoother transitions
		AnimInstance->Montage_Play(AttackMontage, 1.0f);
		
		// Cycle through attack animations
		FName SectionName;
		switch (AttackCount)
		{
			case 0:
				SectionName = FName("Attack1");
				break;
			case 1:
				SectionName = FName("Attack2");
				break;
			case 2:
				SectionName = FName("Attack3");
				EnableKickCollision();
				break;
			default:
				SectionName = FName("Attack4");
				AttackCount = 0;
				EnableKickCollision();
				break;
		}
		
		// Use blend time for smoother section transitions
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
		AttackCount++;

		// Bind the montage end delegate
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AMyCharacter::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
	}
}

void AMyCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		AttackEnd();
	}
}

void AMyCharacter::OnKickBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		// Apply damage to the hit actor
		UGameplayStatics::ApplyDamage(
			OtherActor,
			15.f, // Kick damage
			GetController(),
			this,
			UDamageType::StaticClass()
		);

		// Trigger hit reaction
		IHitInterface* HitInterface = Cast<IHitInterface>(OtherActor);
		if (HitInterface)
		{
			HitInterface->GetHit(SweepResult.ImpactPoint);
		}
	}
}

void AMyCharacter::EnableKickCollision()
{
	if (KickBox)
	{
		KickBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void AMyCharacter::DisableKickCollision()
{
	if (KickBox)
	{
		KickBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AMyCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
	DisableKickCollision();
	
	// Disable weapon collision when attack ends
	if (WeaponBox)
	{
		WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// Reset attack count if we've completed the full combo
	if (AttackCount >= 4)
	{
		AttackCount = 0;
	}

	// Reset character movement settings
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
		GetCharacterMovement()->bUseRVOAvoidance = true;
	}

	// Stop any playing montages with a blend out
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.2f);
	}
}

float AMyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (Attributes && Attributes->IsAlive())
    {
        Attributes->ReceiveDamage(DamageAmount);
        
    }
    return DamageAmount;
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update character state based on movement
	if (GetCharacterMovement())
	{
		if (GetCharacterMovement()->IsFalling())
		{
			CharacterState = ECharacterState::ECS_Unequipped;
		}
		else if (GetVelocity().Size() > 0.f)
		{
			CharacterState = ECharacterState::ECS_Equipped;
		}
	}
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMyCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyCharacter::LookUp);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacter::Jump);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMyCharacter::Attack);
}

// Add new functions to enable/disable weapon collision
void AMyCharacter::EnableWeaponCollision()
{
	if (WeaponBox)
	{
		WeaponBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	}
}

void AMyCharacter::DisableWeaponCollision()
{
	if (WeaponBox)
	{
		WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}





	



