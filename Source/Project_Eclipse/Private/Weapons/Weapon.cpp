// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon.h"
#include "Characters/MyCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/HitInterface.h"

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

    // Check if the other actor is our owner (prevent self-damage)
    if (OtherActor == GetOwner())
    {
        return;
    }

    // Check if we've already hit this actor in this attack
    if (HitActors.Contains(OtherActor))
    {
        return;
    }

    // If our owner is a character, also de-duplicate against their per-attack list
    if (AActor* OwnerActor = GetOwner())
    {
        if (AMyCharacter* OwnerChar = Cast<AMyCharacter>(OwnerActor))
        {
            if (OwnerChar->HasAlreadyHit(OtherActor))
            {
                return;
            }
        }
    }

    FHitResult BoxHit;
    bool bHit = BoxTrace(BoxHit);

    if (BoxHit.GetActor())
    {
        // Add this actor to our hit list to prevent multiple hits
        HitActors.Add(OtherActor);
        if (AActor* OwnerActor = GetOwner())
        {
            if (AMyCharacter* OwnerChar = Cast<AMyCharacter>(OwnerActor))
            {
                OwnerChar->RecordHit(OtherActor);
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
        
        UE_LOG(LogTemp, Warning, TEXT("OnBoxOverlap: Hit %s with weapon damage"), *OtherActor->GetName());
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

