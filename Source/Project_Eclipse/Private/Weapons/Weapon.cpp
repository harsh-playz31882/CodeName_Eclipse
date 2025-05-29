// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon.h"
#include "Components/SphereComponent.h"
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

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());



	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
	WeaponBox->SetupAttachment(GetRootComponent());
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	
	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());
	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());

}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	const FString OtherActorName = OtherActor->GetName();
}


void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	const FString OtherActorName = FString("Ending overlap with") + OtherActor->GetName();

}

bool AWeapon::BoxTrace(FHitResult& OutHit, bool bDrawDebug)
{
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
        bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
        OutHit,
        true
    );
}

void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    FHitResult BoxHit;
    bool bHit = BoxTrace(BoxHit, false);

    if (BoxHit.GetActor())
    {
        // Apply damage to the hit actor
        UGameplayStatics::ApplyDamage(
            BoxHit.GetActor(),
            Damage,
            GetInstigator()->GetController(),
            this,
            UDamageType::StaticClass()
        );

        // Handle hit interface if implemented
        if (IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor()))
        {
            HitInterface->GetHit(BoxHit.ImpactPoint);
        }
    }
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

