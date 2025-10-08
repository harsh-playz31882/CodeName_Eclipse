// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon.h"
#include "Characters/MyCharacter.h"
#include "Enemy/Enemy.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/HitInterface.h"
#include "Engine/Engine.h"
#include "Characters/CharacterTypes.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
	RootComponent = SwordMesh;

	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
	WeaponBox->SetupAttachment(GetRootComponent());
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
	WeaponBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	
	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());
	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());

}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
    WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
    // on-screen debug removed

}

bool AWeapon::BoxTrace(FHitResult& OutHit)
{
    if (!BoxTraceStart || !BoxTraceEnd)
    {
        return false;
    }

    const FVector Start = BoxTraceStart->GetComponentLocation();
    const FVector End = BoxTraceEnd->GetComponentLocation();
    const FVector BoxHalfSize = FVector(5.f, 5.f, 5.f);

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);
    // Also ignore the weapon owner and anything attached to the owner to prevent self-hits
    if (AActor* OwnerActor = GetOwner())
    {
        ActorsToIgnore.Add(OwnerActor);

        // Collect all actors attached to the owner recursively (e.g., mesh, sockets, child actors)
        TSet<AActor*> RecursiveAttached;
        GatherAttachedActorsRecursive(OwnerActor, RecursiveAttached);
        for (AActor* AttachedActor : RecursiveAttached)
        {
            if (AttachedActor && AttachedActor != OwnerActor)
            {
                ActorsToIgnore.Add(AttachedActor);
            }
        }
    }

    return UKismetSystemLibrary::BoxTraceSingle(
        this,
        Start,
        End,
        BoxHalfSize,
        BoxTraceStart->GetComponentRotation(),
        ETraceTypeQuery::TraceTypeQuery1,
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        OutHit,
        true
    );
}

void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Check if we have a valid actor and it's not ourselves
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    // Require a valid owner and ensure the owner is currently attacking.
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        // on-screen debug removed
        return;
    }

    // Early-out if owner is not in Attacking action state
    if (const AMyCharacter* OwnerChar = Cast<AMyCharacter>(OwnerActor))
    {
        if (OwnerChar->GetActionState() != EActionState::EAS_Attacking)
        {
            // on-screen debug removed
            return;
        }
    }
    else if (const AEnemy* OwnerEnemy = Cast<AEnemy>(OwnerActor))
    {
        if (OwnerEnemy->ActionState != AEnemy::EActionState::EAS_Attacking)
        {
            // on-screen debug removed
            return;
        }
    }

    // Check if the other actor is our owner (prevent self-damage)
    if (OtherActor == OwnerActor)
    {
        return;
    }

    // Ignore anything attached to the same owner (e.g., owner's mesh, child actors, equipment)
    if (OwnerActor)
    {
        // If the other actor is attached to our owner, ignore
        if (OtherActor->IsAttachedTo(OwnerActor))
        {
            return;
        }
        // If our owner is attached to the other actor (rare), also ignore
        if (OwnerActor->IsAttachedTo(OtherActor))
        {
            return;
        }
        // If the overlapping component belongs to our owner (component-level self overlap), ignore
        if (OtherComp && OtherComp->GetOwner() == OwnerActor)
        {
            return;
        }

        // If OtherActor is any descendant attachment of our owner, ignore
        TSet<AActor*> RecursiveAttached;
        GatherAttachedActorsRecursive(OwnerActor, RecursiveAttached);
        if (RecursiveAttached.Contains(OtherActor))
        {
            return;
        }
    }

    // Check if we've already hit this actor in this attack
    if (HitActors.Contains(OtherActor))
    {
        return;
    }

    // If our owner is a character, also de-duplicate against their per-attack list
    if (OwnerActor)
    {
        if (AMyCharacter* OwnerChar = Cast<AMyCharacter>(OwnerActor))
        {
            if (OwnerChar->HasAlreadyHit(OtherActor))
            {
                return;
            }
        }
        else if (AEnemy* OwnerEnemy = Cast<AEnemy>(OwnerActor))
        {
            if (OwnerEnemy->HasAlreadyHit(OtherActor))
            {
                return;
            }
        }
    }

    FHitResult BoxHit;
    bool bHit = BoxTrace(BoxHit);

    if (bHit && BoxHit.GetActor())
    {
        // Extra safety: ignore owner and attachments from trace result as well
        if (OwnerActor)
        {
            if (BoxHit.GetActor() == OwnerActor || BoxHit.GetActor()->IsAttachedTo(OwnerActor) || OwnerActor->IsAttachedTo(BoxHit.GetActor()))
            {
                return;
            }

            TSet<AActor*> RecursiveAttached;
            GatherAttachedActorsRecursive(OwnerActor, RecursiveAttached);
            if (RecursiveAttached.Contains(BoxHit.GetActor()))
            {
                return;
            }
        }

        // Add this actor to our hit list to prevent multiple hits
        HitActors.Add(OtherActor);
        if (OwnerActor)
        {
            if (AMyCharacter* OwnerChar = Cast<AMyCharacter>(OwnerActor))
            {
                OwnerChar->RecordHit(OtherActor);
            }
            else if (AEnemy* OwnerEnemy = Cast<AEnemy>(OwnerActor))
            {
                OwnerEnemy->RecordHit(OtherActor);
            }
        }
        
        // Apply damage to the hit actor
        AController* DamageInstigatorController = nullptr;
        if (APawn* InstPawn = GetInstigator())
        {
            DamageInstigatorController = InstPawn->GetController();
        }
        else if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            DamageInstigatorController = OwnerPawn->GetController();
        }

        // on-screen debug removed

        UGameplayStatics::ApplyDamage(
            BoxHit.GetActor(),
            Damage,
            DamageInstigatorController,
            this,
            UDamageType::StaticClass()
        );

        // Handle hit interface if implemented
        if (IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor()))
        {
            HitInterface->GetHit(BoxHit.ImpactPoint);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("OnBoxOverlap: Hit %s with weapon damage"), *BoxHit.GetActor()->GetName());
    }
}

void AWeapon::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Remove from per-weapon hit list so repeated valid hits can register across separate swings
    if (OtherActor)
    {
        HitActors.Remove(OtherActor);
    }
}

void AWeapon::GatherAttachedActorsRecursive(AActor* RootActor, TSet<AActor*>& OutAttached)
{
    if (!RootActor || OutAttached.Contains(RootActor))
    {
        return;
    }

    TArray<AActor*> DirectChildren;
    RootActor->GetAttachedActors(DirectChildren);
    for (AActor* Child : DirectChildren)
    {
        if (Child && !OutAttached.Contains(Child))
        {
            OutAttached.Add(Child);
            GatherAttachedActorsRecursive(Child, OutAttached);
        }
    }
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::ClearHitActors()
{
	HitActors.Empty();
	UE_LOG(LogTemp, Warning, TEXT("Weapon: Hit actors list cleared"));
}

