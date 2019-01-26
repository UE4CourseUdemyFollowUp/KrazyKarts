// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"


// Sets default values
AGoKart::AGoKart()
	: Mass(1000.f)
	, MaxDrivingForce(10000.f)
	, MaxDegreesPerSecond(90.f)
	, DragCoefficient(16.f)
	, RollingResistanceCoefficient(0.015f)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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

	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / (Mass > 0.f ? Mass : 1.f);
	


	Velocity += Acceleration * DeltaTime;

	ApplyRotation(DeltaTime);
	UpdateLocationFromVelocity(DeltaTime);

}

void AGoKart::ApplyRotation(float DeltaTime)
{
	float RotationDelta = MaxDegreesPerSecond * DeltaTime * SteeringThrow;
	FQuat DeltaRot(GetActorUpVector(), FMath::DegreesToRadians(RotationDelta));

	Velocity = DeltaRot.RotateVector(Velocity);

	AddActorWorldRotation(DeltaRot, true);
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult* Hit = nullptr;
	AddActorWorldOffset(Translation, true, Hit);

	if (Hit && Hit->bBlockingHit)
	{
		Velocity = FVector::ZeroVector;
	}
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
	Throttle = Val;
}

void AGoKart::MoveRight(float Val)
{
	SteeringThrow = Val;
}

FVector AGoKart::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector AGoKart::GetRollingResistance()
{
	const float AccelerationDueToGravity = -(GetWorld()->GetGravityZ() / 100.f);
	const float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}
