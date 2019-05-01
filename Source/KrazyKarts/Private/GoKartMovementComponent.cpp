// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementComponent.h"

// Sets default values for this component's properties
UGoKartMovementComponent::UGoKartMovementComponent()
: Mass(1000.f)
, MaxDrivingForce(10000.f)
, MinTurningRadius(10.f)
, DragCoefficient(16.f)
, RollingResistanceCoefficient(0.015f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGoKartMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGoKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGoKartMovementComponent::ApplyRotation(const float& DeltaTime, const float& SteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationDelta = DeltaLocation / MinTurningRadius * SteeringThrow;
	FQuat DeltaRot(GetOwner()->GetActorUpVector(), RotationDelta);

	Velocity = DeltaRot.RotateVector(Velocity);

	GetOwner()->AddActorWorldRotation(DeltaRot, true);
}

void UGoKartMovementComponent::UpdateLocationFromVelocity(const float& DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult* Hit = nullptr;
	GetOwner()->AddActorWorldOffset(Translation, true, Hit);

	if (Hit && Hit->bBlockingHit)
	{
		Velocity = FVector::ZeroVector;
	}
}

void UGoKartMovementComponent::SimulateMove(const FGoKartMove & Move)
{
	FVector Force = GetOwner()->GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
	UE_LOG(LogTemp, Warning, TEXT("Force calculation: %d,%d"), Force.X, Force.Y);
	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / (Mass > 0.f ? Mass : 1.f);

	Velocity += Acceleration * Move.DeltaTime;

	ApplyRotation(Move.DeltaTime, Move.SteeringThrow);
	UpdateLocationFromVelocity(Move.DeltaTime);
}

FGoKartMove UGoKartMovementComponent::CreateMove(const float & DeltaTime) const
{
	FGoKartMove Move;
	Move.DeltaTime = DeltaTime;
	Move.Throttle = Throttle;
	Move.SteeringThrow = SteeringThrow;
	//Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	Move.Time = GetWorld()->GetTimeSeconds();

	return Move;
}

FVector UGoKartMovementComponent::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector UGoKartMovementComponent::GetRollingResistance()
{
	const float AccelerationDueToGravity = -(GetWorld()->GetGravityZ() / 100.f);
	const float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}
