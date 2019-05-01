// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine/Classes/GameFramework/GameStateBase.h"

FString GetEnumText(const ENetRole& role)
{
	FString Ret = "";
	switch (role)
	{
	case ENetRole::ROLE_Authority:
		Ret = "ROLE_Authority";
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Ret = "ROLE_AutonomousProxy";
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Ret = "ROLE_SimulatedProxy";
		break;
	default:
		Ret = "Unknown";
		break;
	}

	return Ret;
}

// Sets default values
AGoKart::AGoKart()
	/*: Mass(1000.f)
	, MaxDrivingForce(10000.f)
	, MinTurningRadius(10.f)
	, DragCoefficient(16.f)
	, RollingResistanceCoefficient(0.015f)*/
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bReplicateMovement = false;

	if (HasAuthority())
	{
		NetUpdateFrequency = 1.f;
	}

	MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("MovementComponent"));
	MovementReplicator = CreateDefaultSubobject<UGoKartMovementReplicator>(TEXT("MovementReplicator"));
	
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!MovementComponent || !MovementReplicator)
		return;

	DrawDebugString(GetWorld(), FVector(0.f, 0.f, 100.f), GetEnumText(Role), this, FColor::White, DeltaTime);

}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::MoveForward(float Val)
{
	if (!MovementComponent)
		return;

	MovementComponent->SetThrottle(Val);
}

void AGoKart::MoveRight(float Val)
{
	if (!MovementComponent)
		return;

	MovementComponent->SetSteeringThrow(Val);
}
