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
#include "HUD/MainHUD.h"
#include "HUD/Character_Overlay.h"

#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"




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

    // Create kick box components for both legs
    KickBoxLeft = CreateDefaultSubobject<UBoxComponent>(TEXT("Kick Box Left"));
    KickBoxLeft->SetupAttachment(GetMesh(), FName("foot_l"));
    KickBoxLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    KickBoxLeft->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    KickBoxLeft->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

    KickBoxRight = CreateDefaultSubobject<UBoxComponent>(TEXT("Kick Box Right"));
    KickBoxRight->SetupAttachment(GetMesh(), FName("foot_r"));
    KickBoxRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    KickBoxRight->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    KickBoxRight->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

    // Set up montages
    AttackMontage = CreateDefaultSubobject<UAnimMontage>(TEXT("/Game/Animations/KYRA2_Animations/AM_AttackMontage3.AM_AttackMontage3"));
    HitReactMontage = CreateDefaultSubobject<UAnimMontage>(TEXT("/Game/Animations/KYRA2_Animations/HitReact/AM_HitReact3.AM_HitReact3"));
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}



	InitializeCharacterOverlay();
	GetCharacterMovements();
	StopMontages();

	USkeletalMeshComponent* MeshComp = GetMesh(); 

	if (MeshComp && MeshComp->GetSkeletalMeshAsset())
	{
		if (MeshComp->DoesSocketExist(FName("WeaponSocket")))
		{
			FVector SocketLocation = MeshComp->GetSocketLocation(FName("WeaponSocket"));
		}
		
	}
	
	if (WeaponBox && GetMesh())
	{
		WeaponBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("weapon_r"));
		WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	}

	
	KickBoxLeft->OnComponentBeginOverlap.AddDynamic(this, &AMyCharacter::OnKickBoxOverlap);
	KickBoxRight->OnComponentBeginOverlap.AddDynamic(this, &AMyCharacter::OnKickBoxOverlap);

	Tags.Add(FName("MyCharacter"));

}

void AMyCharacter::StopMontages()
{
	// Stop any playing montages
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.0f);
	}
}

void AMyCharacter::GetCharacterMovements()
{
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

void AMyCharacter::Jump()
{
	if (ActionState == EActionState::EAS_Unoccupied)
	{
		// Call the parent class jump function
		Super::Jump();
		
		// You can add additional jump logic here if needed
		// For example, play jump sound, particles, or animations
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
    if (KickBoxLeft)
    {
        KickBoxLeft->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
    if (KickBoxRight)
    {
        KickBoxRight->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }
}

void AMyCharacter::DisableKickCollision()
{
    if (KickBoxLeft)
    {
        KickBoxLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (KickBoxRight)
    {
        KickBoxRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
        
        // Update the health bar UI
        if (Character_Overlay)
        {
            Character_Overlay->SetHealthBarPercent(Attributes->GetHealthPercent());
        }
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

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind the movement action
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AMyCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AMyCharacter::Jump);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AMyCharacter::Attack);
	}
	
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

void AMyCharacter::InitializeCharacterOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		AMainHUD* MainHUD = Cast<AMainHUD>(PlayerController->GetHUD());
		if (MainHUD)
		{
			Character_Overlay = MainHUD->GetCharacterOverlay();
			if (Character_Overlay && Attributes)
			{
				Character_Overlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				Character_Overlay->SetStaminaBarPercent(1.f);
			}

		}
	}
}

void AMyCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AMyCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void AMyCharacter::GetHit(const FVector& ImpactPoint)
{
    if (!GetMesh() || !GetMesh()->GetAnimInstance())
    {
        UE_LOG(LogTemp, Warning, TEXT("GetHit: Mesh or AnimInstance is null"));
        return;
    }

    if (Attributes && Attributes->IsAlive())
    {
        // Stop any current montages
        StopMontages();

        // Play hit reaction
        DirectionalHitReact(ImpactPoint);
        UE_LOG(LogTemp, Warning, TEXT("GetHit: Playing hit reaction"));

        // Disable weapon collision during hit reaction
        DisableWeaponCollision();
        DisableKickCollision();
    }

    // Play hit effects
    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
    }
    if (HitParticles && GetWorld())
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            HitParticles,
            ImpactPoint
        );
    }
}

void AMyCharacter::OnHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!Montage) return;
    
    if (Montage == HitReactMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnHitReactMontageEnded: Hit reaction montage ended"));
        ActionState = EActionState::EAS_Unoccupied;
        
        // Re-enable movement rotation
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->bOrientRotationToMovement = true;
            GetCharacterMovement()->bUseControllerDesiredRotation = false;
        }
    }
}





	



