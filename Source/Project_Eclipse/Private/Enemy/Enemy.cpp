#include "Enemy/Enemy.h"
#include "AIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/Weapon.h"




AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Set up mesh collision
	if (GetMesh())
	{
		GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
		GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
		GetMesh()->SetGenerateOverlapEvents(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		UE_LOG(LogTemp, Warning, TEXT("Enemy: Mesh collision setup complete"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy: Mesh is null during construction"));
	}

	// Set up capsule collision
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
		UE_LOG(LogTemp, Warning, TEXT("Enemy: Capsule collision setup complete"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy: Capsule component is null during construction"));
	}

	// Create and set up health bar widget
	HealthBarWidget1 = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBarWidget1"));
	HealthBarWidget1->SetupAttachment(GetRootComponent());

	// Set up character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->bUseRVOAvoidance = true;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;

	// Disable controller rotation
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Create AI perception component
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	// Set up weapon box collision
	if (WeaponBox)
	{
		WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
		WeaponBox->IgnoreActorWhenMoving(this, true);
		WeaponBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	}
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Enemy BeginPlay started"));

	if (HealthBarWidget1)
	{
		HealthBarWidget1->SetHealthPercent(1.f);
		UE_LOG(LogTemp, Warning, TEXT("Health bar widget initialized"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Health bar widget is null"));
	}

	EnemyController = Cast<AAIController>(GetController());
	if (!EnemyController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy controller is null"));
		return;
	}

	if (!PatrolTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Patrol target is null"));
		return;
	}

	// Initialize patrol state
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;

	// Set up AI perception
	if (AIPerception)
	{
		AIPerception->OnPerceptionUpdated.AddDynamic(this, &AEnemy::OnPerceptionUpdated);
		UE_LOG(LogTemp, Warning, TEXT("AI perception initialized"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AI perception is null"));
	}

	// Set up weapon collision
	if (WeaponBox)
	{
		WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnWeaponBoxOverlap);
		UE_LOG(LogTemp, Warning, TEXT("Weapon box overlap delegate bound"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon box is null"));
	}

	// Start patrolling
	MoveToTarget(PatrolTarget);
	UE_LOG(LogTemp, Warning, TEXT("Enemy BeginPlay completed"));
}

void AEnemy::WeaponBoxCollision()
{
	if (WeaponBox)
	{
		WeaponBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		// Only detect player pawn
		WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		// Ignore self collision
		WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
		// Make sure to ignore the enemy's own collision
		WeaponBox->IgnoreActorWhenMoving(this, true);
		// Ignore the enemy's own mesh
		WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
		// Set collision object type
		WeaponBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	}
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (!EnemyController)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToTarget: Enemy controller is null"));
		return;
	}

	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToTarget: Target is null"));
		return;
	}

	// Check if we're already moving to this target
	if (EnemyController->GetMoveStatus() == EPathFollowingStatus::Moving)
	{
		FVector CurrentGoal = EnemyController->GetPathFollowingComponent()->GetPathDestination();
		if (FVector::DistSquared(CurrentGoal, Target->GetActorLocation()) < 10000.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("MoveToTarget: Already moving to target"));
			return;
		}
	}

	// Stop any existing movement
	EnemyController->StopMovement();

	// Start new movement
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(100.f);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(true);

	EnemyController->MoveTo(MoveRequest);
	UE_LOG(LogTemp, Warning, TEXT("MoveToTarget: Moving to new target"));
}

void AEnemy::Attack()
{
	if (ActionState != EActionState::EAS_Unoccupied)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack: Already attacking"));
		return;
	}

	if (!GetMesh() || !GetMesh()->GetAnimInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack: Mesh or AnimInstance is null"));
		return;
	}

	if (!AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack: AttackMontage is null"));
		return;
	}

	Super::Attack();
	ActionState = EActionState::EAS_Attacking;

	// Face the player
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		FVector Direction = PlayerPawn->GetActorLocation() - GetActorLocation();
		Direction.Z = 0.f;
		FRotator NewRotation = Direction.Rotation();
		SetActorRotation(NewRotation);
		UE_LOG(LogTemp, Warning, TEXT("Attack: Facing player"));
	}

	// Play attack montage
	PlayAttackMontage();
}

void AEnemy::AttackEnd()
{
	Super::AttackEnd();
	ActionState = EActionState::EAS_Unoccupied;

	// Disable weapon collision
	if (WeaponBox)
	{
		WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AEnemy::PlayAttackMontage()
{
	if (!GetMesh() || !GetMesh()->GetAnimInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayAttackMontage: Mesh or AnimInstance is null"));
		return;
	}

	if (!AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayAttackMontage: AttackMontage is null"));
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	// Don't stop existing montages if we're already attacking
	if (ActionState != EActionState::EAS_Attacking)
	{
		AnimInstance->StopAllMontages(0.1f);
	}

	// Set montage blend settings for smoother transitions
	AnimInstance->Montage_Play(AttackMontage, 1.0f);
	
	// Always play the first attack when starting a new sequence
	FName SectionName = FName("Attack1");
	AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
	
	// Bind the montage end delegate
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &AEnemy::OnAttackMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);

	UE_LOG(LogTemp, Warning, TEXT("PlayAttackMontage: Started attack montage"));
}

void AEnemy::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;

	if (Montage == AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnAttackMontageEnded: Attack montage ended"));
		ActionState = EActionState::EAS_Unoccupied;
		
		if (WeaponBox)
		{
			WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			UE_LOG(LogTemp, Warning, TEXT("OnAttackMontageEnded: Weapon collision disabled"));
		}
		
		AttackCount = 0;
	}
}

AActor* AEnemy::ChoosePatrolTarget()
{
	TArray<AActor*> ValidTargets;
	for (AActor* Target : PatrolTargets)
	{
		if (Target != PatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}
	const int32 NumPatrolTargets = ValidTargets.Num();
	if (NumPatrolTargets > 0)
	{
		const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
		return  ValidTargets[TargetSelection];

	}
	return nullptr;
}

void AEnemy::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);

	}
}



void AEnemy::Die()
{
	// Set the death flag
	bIsDead = true;

	// Stop any existing movement and AI
	if (EnemyController)
	{
		EnemyController->StopMovement();
		EnemyController->UnPossess();
	}

	// Disable collision and movement
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	//playing death montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	FName SectionName = FName();
	UAnimMontage* MontageToUse = DeathMontage;
	
	if (AnimInstance && MontageToUse)
	{
		// Stop any existing montages first
		AnimInstance->StopAllMontages(0.0f);
		
		const int32 Selection = FMath::RandRange(0, 1);
		switch (Selection)
		{
		case 0:
			SectionName = FName("flying_death");
			DeathPose = EDeathPose::EDP_Death1;
			break;
		case 1:
			SectionName = FName("standing_death");
			DeathPose = EDeathPose::EDP_Death2;
			break;
		default:
			break;
		}

		// Play the death animation
		AnimInstance->Montage_Play(DeathMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		AnimInstance->Montage_JumpToSection(SectionName, DeathMontage);

		// Get the length of the current section
		float SectionLength = DeathMontage->GetSectionLength(DeathMontage->GetSectionIndex(SectionName));
		
		// Set up a timer to jump to the last frame section
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, AnimInstance, MontageToUse, SectionName, SectionLength]()
		{
			if (AnimInstance && DeathMontage)
			{
				// Stop the montage and jump to the specific last frame section
				AnimInstance->Montage_Stop(0.0f, DeathMontage);
				AnimInstance->Montage_JumpToSection(SectionName, DeathMontage);
				AnimInstance->Montage_SetPosition(DeathMontage, SectionLength);
				
				// Disable animation updates to freeze the pose
				GetMesh()->bPauseAnims = true;
			}
		}, SectionLength, false); // Wait for the full animation to play
	}

	// Disable weapon collision
	if (WeaponBox)
	{
		WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::GetHit(const FVector& ImpactPoint)
{
	
	/*
	UWorld* World = GetWorld();
	FVector Location = GetActorLocation();

	if (World)
	{
		DrawDebugSphere(World, Location, 10.f, 24, FColor::Red, false, 3.f);
	}
	*/
	
	
	if (Attributes && Attributes->IsAlive()) 
	{
		DirectionalHitReact(ImpactPoint);
	}
	else
	{
		Die();

	}
	

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

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Attributes && HealthBarWidget1)
	{
		Attributes->ReceiveDamage(DamageAmount);
		HealthBarWidget1->SetHealthPercent(Attributes->GetHealthPercent());
		
		// Only update state and movement if we're not currently attacking
		if (ActionState != EActionState::EAS_Attacking)
		{
			EnemyState = EEnemyState::EES_Chasing;
			GetCharacterMovement()->MaxWalkSpeed = 300.f;
			
			// Only update target if we're not already moving to it
			if (DamageCauser && (EnemyController == nullptr || 
				EnemyController->GetMoveStatus() != EPathFollowingStatus::Moving ||
				EnemyController->GetPathFollowingComponent()->GetPathDestination() != DamageCauser->GetActorLocation()))
			{
				MoveToTarget(DamageCauser);
			}
		}
	}
	return DamageAmount;
}

void AEnemy::PatrolTimerFinished()
{
	UE_LOG(LogTemp, Warning, TEXT("Patrol timer finished"));
	
	if (!EnemyController)
	{
		UE_LOG(LogTemp, Warning, TEXT("PatrolTimerFinished: Enemy controller is null"));
		return;
	}

	if (!PatrolTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("PatrolTimerFinished: Patrol target is null"));
		return;
	}

	MoveToTarget(PatrolTarget);
}

void AEnemy::CheckPatroTarget()
{
	if (EnemyState == EEnemyState::EES_Patrolling)
	{
		if (InTargetRange(PatrolTarget, PatrolRadius))
		{
			PatrolTarget = ChoosePatrolTarget();
			MoveToTarget(PatrolTarget);
		}
	}
}

void AEnemy::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	bool bPlayerSeen = false;
	for (AActor* Actor : UpdatedActors)
	{
		APawn* PlayerPawn = Cast<APawn>(Actor);
		if (PlayerPawn)
		{
			bPlayerSeen = true;
			// Chase the player
			EnemyState = EEnemyState::EES_Chasing;
			MoveToTarget(PlayerPawn);
		}
	}
	
	// If no player is seen, go back to patrolling
	if (!bPlayerSeen)
	{
		EnemyState = EEnemyState::EES_Patrolling;
		CheckPatroTarget();
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyState == EEnemyState::EES_Dead) return;

	if (EnemyState == EEnemyState::EES_Patrolling)
	{
		if (PatrolTarget && InTargetRange(PatrolTarget, PatrolRadius))
		{
			// Start timer to move to next patrol point
			GetWorldTimerManager().SetTimer(
				PatrolTimer,
				this,
				&AEnemy::PatrolTimerFinished,
				FMath::RandRange(WaitMin, WaitMax)
			);
			UE_LOG(LogTemp, Warning, TEXT("Tick: Started patrol timer"));
		}
	}

	if (bIsDead) return;

	// Get the player pawn
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return;

	// Check if we're in attack range
	if (InTargetRange(PlayerPawn, AttackRange))
	{
		if (EnemyController)
		{
			EnemyController->StopMovement();
		}

		if (ActionState == EActionState::EAS_Unoccupied)
		{
			Attack();
		}
	}
	else
	{
		// Move to target if we're not in range
		MoveToTarget(PlayerPawn);
	}
}

void AEnemy::OnWeaponBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Only process if we're in attacking state
	if (ActionState != EActionState::EAS_Attacking) return;

	// Check if the other actor is the player and not self
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (OtherActor == PlayerPawn && OtherActor != this)
	{
		// Apply damage to the player
		UGameplayStatics::ApplyDamage(
			PlayerPawn,
			AttackDamage,
			GetController(),
			this,
			UDamageType::StaticClass()
		);

		// Trigger hit reaction if the player implements the hit interface
		if (IHitInterface* HitInterface = Cast<IHitInterface>(PlayerPawn))
		{
			HitInterface->GetHit(SweepResult.ImpactPoint);
		}

		// Disable weapon collision after hit to prevent multiple hits
		if (WeaponBox)
		{
			WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

