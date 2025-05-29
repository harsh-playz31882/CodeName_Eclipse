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
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);


	HealthBarWidget1 = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBarWidget1"));
	HealthBarWidget1->SetupAttachment(GetRootComponent());


	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;
	
	// Check if we're already moving to this target
	if (EnemyController->GetMoveStatus() == EPathFollowingStatus::Moving)
	{
		// Only update if the target has moved significantly
		FVector CurrentGoal = EnemyController->GetPathFollowingComponent()->GetPathDestination();
		if (FVector::DistSquared(CurrentGoal, Target->GetActorLocation()) < 10000.f) // 100 units squared
		{
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
}

void AEnemy::Attack()
{
	Super::Attack();
	
	// Set action state to attacking
	ActionState = EActionState::EAS_Attacking;

	// Enable weapon collision with proper settings
	if (WeaponBox)
	{
		WeaponBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		// Ignore self collision
		WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
	}

	// Ensure the enemy is facing the player
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		FVector Direction = PlayerPawn->GetActorLocation() - GetActorLocation();
		Direction.Z = 0.f;
		FRotator NewRotation = Direction.Rotation();
		SetActorRotation(NewRotation);
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
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		const int32 Selection = FMath::RandRange(0, 2); 
		FName SectionName = FName();
		switch (Selection)
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
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);

		// Bind the montage end delegate with correct syntax
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AEnemy::OnAttackEnd);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
	}
}

void AEnemy::OnAttackEnd(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		AttackEnd();
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

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("=== ENEMY BEGIN PLAY ==="));

	if (HealthBarWidget1)
	{
		HealthBarWidget1->SetHealthPercent(1.f);
	}

	EnemyController = Cast<AAIController>(GetController());
	if (EnemyController && PatrolTarget)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(PatrolTarget);
		MoveRequest.SetAcceptanceRadius(60.f);
		FNavPathSharedPtr NavPath;
		EnemyController->MoveTo(MoveRequest, &NavPath);
		TArray<FNavPathPoint>& PathPoints = NavPath->GetPathPoints();
	}
	
	// Set initial state to chasing
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;

	// Find and chase the player immediately
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		MoveToTarget(PlayerPawn);
	}

	if (AIPerception)
	{
		AIPerception->OnPerceptionUpdated.AddDynamic(this, &AEnemy::OnPerceptionUpdated);
	}

	// Set up weapon box collision
	if (WeaponBox)
	{
		WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	}
}

void AEnemy::Die()
{
	// Set the death flag
	bIsDead = true;

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
		AnimInstance->Montage_Play(MontageToUse, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		AnimInstance->Montage_JumpToSection(SectionName, MontageToUse);

		// Get the length of the current section
		float SectionLength = MontageToUse->GetSectionLength(MontageToUse->GetSectionIndex(SectionName));
		
		// Set up a timer to jump to the last frame section
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, AnimInstance, MontageToUse, SectionName, SectionLength]()
		{
			if (AnimInstance && MontageToUse)
			{
				// Stop the montage and jump to the specific last frame section
				AnimInstance->Montage_Stop(0.0f, MontageToUse);
				AnimInstance->Montage_JumpToSection(SectionName, MontageToUse);
				AnimInstance->Montage_SetPosition(MontageToUse, SectionLength);
			}
		}, SectionLength, false); // Wait for the full animation to play
	}

	// Stop movement and disable AI
	if (EnemyController)
	{
		EnemyController->StopMovement();
		EnemyController->UnPossess();
	}

	// Disable collision and movement
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
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
		HealthBarWidget1->SetHealthPercent(Attributes->GetHealthPerecent());
		
		// Always update state and movement when taking damage
		EnemyState = EEnemyState::EES_Chasing;
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
		
		// Reset action state to allow new attacks
		ActionState = EActionState::EAS_Unoccupied;
		
		// Only update target if we're not already moving to it
		if (DamageCauser && (EnemyController == nullptr || 
			EnemyController->GetMoveStatus() != EPathFollowingStatus::Moving ||
			EnemyController->GetPathFollowingComponent()->GetPathDestination() != DamageCauser->GetActorLocation()))
		{
			MoveToTarget(DamageCauser);
		}
	}
	return DamageAmount;
}

void AEnemy::PatrolTimerFinished()
{
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
			// Play attack montage regardless of range
			PlayAttackMontage();
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

	if (bIsDead) return;

	// Get the player pawn
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return;

	// Check if we're in attack range
	if (InTargetRange(PlayerPawn, AttackRange))
	{
		// Stop movement when in attack range
		if (EnemyController)
		{
			EnemyController->StopMovement();
		}

		// Attack if we're not already attacking
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





	/*
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + CrossProduct * 100.f, 5.f, FColor::Blue, 3.f);


	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Green, FString::Printf(TEXT("Theta: %f"), Theta));
	}

	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + Forward * 60.f, 5.f, FColor::Red, 3.f);
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + ToHit * 60.f, 5.f, FColor::Green, 3.f);
*/
