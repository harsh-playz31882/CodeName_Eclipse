#include "Characters/MyCharacter.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/BoxComponent.h"
#include "Weapons/Weapon.h"
#include "Components/AttributeComponent.h"
#include "HUD/MyHUD.h"
#include "HUD/MainHUD.h"
#include "Enemy/Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/HitInterface.h"
#include "Perception/AIPerceptionComponent.h"
#include "HUD/Character_Overlay.h"

#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"




AMyCharacter::AMyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Camera boom and follow camera
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->TargetArmLength = 300.f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bInheritPitch = true;
    CameraBoom->bInheritYaw = true;
    CameraBoom->bInheritRoll = false;

    ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
    ViewCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    ViewCamera->bUsePawnControlRotation = false;
    ViewCamera->FieldOfView = 90.f;


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

    // Montages should be set in Blueprint editor, not in constructor
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
		if (MeshComp->DoesSocketExist(FName("RightHandSocket")))
		{
			FVector SocketLocation = MeshComp->GetSocketLocation(FName("RightHandSocket"));
		}
		
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


void AMyCharacter::Attack()
{
    // Keep for compatibility if needed: default to Attack1
    Attack1();
}

void AMyCharacter::PlayAttackMontage()
{
    // Legacy path: default to first section
    PlayAttackMontageSection(FName("Attack1"));
}

bool AMyCharacter::PlayAttackMontageSection(FName SectionName)
{
    if (ActionState != EActionState::EAS_Unoccupied)
    {
        return false;
    }

    UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!AnimInstance || !AttackMontage)
    {
        return false;
    }

    ActionState = EActionState::EAS_Attacking;

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = false;
        GetCharacterMovement()->bUseControllerDesiredRotation = true;
    }

    HitActors.Empty();
    ClearWeaponHitActors();
    DisableWeaponCollision();
    DisableKickCollision();

    AnimInstance->Montage_Play(AttackMontage, 1.0f);
    AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AMyCharacter::OnMontageEnded);
    AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);

    EnableKickCollision();
    return true;
}

void AMyCharacter::Attack1()
{
    PlayAttackMontageSection(FName("Attack1"));
}

void AMyCharacter::Attack2()
{
    PlayAttackMontageSection(FName("Attack2"));
}

void AMyCharacter::Attack3()
{
    PlayAttackMontageSection(FName("Attack3"));
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

		// Set the hit enemy as the targeted enemy for the HUD
		if (AEnemy* HitEnemy = Cast<AEnemy>(OtherActor))
		{
			if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
			{
				if (AMainHUD* MainHUD = Cast<AMainHUD>(PlayerController->GetHUD()))
				{
					MainHUD->SetTargetedEnemy(HitEnemy);
				}
			}
		}
		
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
        EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AMyCharacter::Move);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMyCharacter::Jump);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacter::Look);

        // Bind specific attack inputs to fixed sections
        EnhancedInputComponent->BindAction(AttackAction1, ETriggerEvent::Triggered, this, &AMyCharacter::Attack1);
        EnhancedInputComponent->BindAction(AttackAction2, ETriggerEvent::Triggered, this, &AMyCharacter::Attack2);
        EnhancedInputComponent->BindAction(AttackAction3, ETriggerEvent::Triggered, this, &AMyCharacter::Attack3);

	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetupPlayerInputComponent: Failed to cast to EnhancedInputComponent"));
	}
}

// Add new functions to enable/disable weapon collision
void AMyCharacter::EnableWeaponCollision()
{
	Super::EnableWeaponCollision();
	UE_LOG(LogTemp, Warning, TEXT("MyCharacter EnableWeaponCollision: Called"));
}

void AMyCharacter::DisableWeaponCollision()
{
	Super::DisableWeaponCollision();
	UE_LOG(LogTemp, Warning, TEXT("MyCharacter DisableWeaponCollision: Called"));
}

void AMyCharacter::ClearWeaponHitActors()
{
	Super::ClearWeaponHitActors();
	UE_LOG(LogTemp, Warning, TEXT("MyCharacter ClearWeaponHitActors: Called"));
}

void AMyCharacter::SetWeaponOwner(AWeapon* Weapon)
{
	if (Weapon)
	{
		Weapon->SetOwner(this);
		Weapon->SetInstigator(this);
		EquippedWeapon = Weapon;
		
		// Initially disable weapon collision
		DisableWeaponCollision();
		
		UE_LOG(LogTemp, Warning, TEXT("MyCharacter SetWeaponOwner: Weapon owner set to %s"), *GetName());
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
			FString::Printf(TEXT("Player Weapon Owner Set: %s"), *GetName()));
		
		// DEBUG: Test if weapon collision can be enabled manually
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, 
			TEXT("DEBUG: Try pressing 'T' to manually enable weapon collision"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MyCharacter SetWeaponOwner: Weapon is null"));
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("MyCharacter SetWeaponOwner: Weapon is null"));
	}
}

void AMyCharacter::TestEnableWeaponCollision()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Purple, TEXT("MANUAL TEST: Enabling weapon collision"));
	EnableWeaponCollision();
	
	if (EquippedWeapon)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Purple, 
			FString::Printf(TEXT("MANUAL TEST: Weapon owner is %s"), EquippedWeapon->GetOwner() ? *EquippedWeapon->GetOwner()->GetName() : TEXT("NULL")));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("MANUAL TEST: No equipped weapon"));
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




