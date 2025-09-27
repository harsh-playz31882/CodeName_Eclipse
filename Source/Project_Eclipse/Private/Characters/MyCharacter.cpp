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
    KickBoxLeft->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    KickBoxLeft->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    KickBoxRight = CreateDefaultSubobject<UBoxComponent>(TEXT("Kick Box Right"));
    KickBoxRight->SetupAttachment(GetMesh(), FName("foot_r"));
    KickBoxRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    KickBoxRight->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    KickBoxRight->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // Set up montages
    AttackMontage = CreateDefaultSubobject<UAnimMontage>(TEXT("/Game/Animations/test_character/AM_Attack.AM_Attack"));
    HitReactMontage = CreateDefaultSubobject<UAnimMontage>(TEXT("/Game/Animations/test_character/AM_HitReact.AM_HitReact"));
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
		
		// Add these settings to prevent animation offset issues
		GetCharacterMovement()->bMaintainHorizontalGroundVelocity = true;
		GetCharacterMovement()->bConstrainToPlane = true;
		GetCharacterMovement()->bSnapToPlaneAtStart = true;
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
		// Clear the hit actors list for this new attack
		HitActors.Empty();
		
		// Clear weapon hit actors as well
		ClearWeaponHitActors();
		
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
				break;
			default:
				SectionName = FName("Attack1");
				AttackCount = 0;
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

void AMyCharacter::PlayAttackSection(const FName& SectionName, bool bEnableKickCollision)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance || !AttackMontage) return;

	// Reset per-attack state
	HitActors.Empty();
	ClearWeaponHitActors();
	DisableWeaponCollision();
	DisableKickCollision();

	// Enter attacking state and lock rotation
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
	}
	ActionState = EActionState::EAS_Attacking;

	AnimInstance->Montage_Play(AttackMontage, 1.0f);
	AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);

	if (bEnableKickCollision)
	{
		EnableKickCollision();
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AMyCharacter::OnMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
}

void AMyCharacter::CrescentKick()
{
	PlayAttackMontage();
}

void AMyCharacter::HurricaneKick()
{
	PlayAttackMontage();
}

void AMyCharacter::SpinAttack()
{
	PlayAttackMontage();
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
	// Check if we have a valid actor and it's not ourselves
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// Check if we're currently attacking
	if (ActionState != EActionState::EAS_Attacking)
	{
		return;
	}

	// Check if the other actor is part of our character (prevent self-damage)
	if (OtherActor == GetOwner() || OtherActor->IsAttachedTo(this))
	{
		return;
	}

	// Check if we've already hit this actor in this attack
	if (HitActors.Contains(OtherActor))
	{
		return;
	}

	// Check if the other actor is alive (implements hit interface)
	IHitInterface* HitInterface = Cast<IHitInterface>(OtherActor);
	if (HitInterface)
	{
		// Add this actor to our hit list to prevent multiple hits
		HitActors.Add(OtherActor);
		
		// Apply damage to the hit actor
		UGameplayStatics::ApplyDamage(
			OtherActor,
			15.f, // Kick damage
			GetController(),
			this,
			UDamageType::StaticClass()
		);

		// Trigger hit reaction
		HitInterface->GetHit(SweepResult.ImpactPoint);
		
		UE_LOG(LogTemp, Warning, TEXT("OnKickBoxOverlap: Hit %s with kick damage"), *OtherActor->GetName());
	}
}

void AMyCharacter::EnableKickCollision()
{
    // Add a small delay to prevent rapid-fire hits
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        if (KickBoxLeft)
        {
            KickBoxLeft->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        if (KickBoxRight)
        {
            KickBoxRight->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        UE_LOG(LogTemp, Warning, TEXT("EnableKickCollision: Kick collision enabled after delay"));
    }, 0.1f, false); // 0.1 second delay
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
	
	// Clear the hit actors list for the next attack
	HitActors.Empty();
	
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
	
	// Continuously control animation root motion to prevent offset issues
	ControlAnimationRootMotion();
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind the movement action
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AMyCharacter::Move);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMyCharacter::Jump);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
	

		// Optional: bind explicit variants if mapped in IMC
		if (CrescentKickAction)
		{
			EnhancedInputComponent->BindAction(CrescentKickAction, ETriggerEvent::Started, this, &AMyCharacter::CrescentKick);
		}
		if (HurricaneKickAction)
		{
			EnhancedInputComponent->BindAction(HurricaneKickAction, ETriggerEvent::Started, this, &AMyCharacter::HurricaneKick);
		}
		if (SpinAttackAction)
		{
			EnhancedInputComponent->BindAction(SpinAttackAction, ETriggerEvent::Started, this, &AMyCharacter::SpinAttack);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetupPlayerInputComponent: Failed to cast to EnhancedInputComponent"));
	}
}

// Add new functions to enable/disable weapon collision
void AMyCharacter::EnableWeaponCollision()
{
	// Add a small delay to prevent rapid-fire hits
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		if (WeaponBox)
		{
			WeaponBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
			WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
		}
		UE_LOG(LogTemp, Warning, TEXT("EnableWeaponCollision: Weapon collision enabled after delay"));
	}, 0.1f, false); // 0.1 second delay
}

void AMyCharacter::DisableWeaponCollision()
{
	if (WeaponBox)
	{
		WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AMyCharacter::ClearWeaponHitActors()
{
	// Clear weapon hit actors if we have a weapon
	if (WeaponBox)
	{
		// Try to get the weapon actor and clear its hit list
		if (AActor* WeaponActor = WeaponBox->GetOwner())
		{
			if (AWeapon* Weapon = Cast<AWeapon>(WeaponActor))
			{
				Weapon->ClearHitActors();
			}
		}
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
	
	// Control animation root motion during movement
	ControlAnimationRootMotion();
}

void AMyCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void AMyCharacter::ControlAnimationRootMotion()
{
	// Simple approach: Just ensure the character stays grounded during movement
	// This prevents the animation offset issue without complex root motion control
	if (GetCharacterMovement())
	{
		// Keep the character properly constrained to the ground plane
		GetCharacterMovement()->bMaintainHorizontalGroundVelocity = true;
		GetCharacterMovement()->bConstrainToPlane = true;
		GetCharacterMovement()->bSnapToPlaneAtStart = true;
	}
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





	



