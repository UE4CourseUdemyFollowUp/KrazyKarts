// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"
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
	: Mass(1000.f)
	, MaxDrivingForce(10000.f)
	, MinTurningRadius(10.f)
	, DragCoefficient(16.f)
	, RollingResistanceCoefficient(0.015f)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	if (HasAuthority())
	{
		NetUpdateFrequency = 1.f;
	}
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

	if (Role == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = CreateMove(DeltaTime);
		UnacknowledgedMoves.Add(Move);
		Server_SendMove(Move);
		SimulateMove(Move);
	}
	else if (Role == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FGoKartMove Move = CreateMove(DeltaTime);
		Server_SendMove(Move);
	}
	else if (Role == ROLE_SimulatedProxy)
	{
		SimulateMove(ServerState.LastMove);
	}

	DrawDebugString(GetWorld(), FVector(0.f, 0.f, 100.f), GetEnumText(Role), this, FColor::White, DeltaTime);

}

void AGoKart::ApplyRotation(const float& DeltaTime, const float& SteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationDelta = DeltaLocation / MinTurningRadius * SteeringThrow;
	FQuat DeltaRot(GetActorUpVector(), RotationDelta);

	Velocity = DeltaRot.RotateVector(Velocity);

	AddActorWorldRotation(DeltaRot, true);
}

void AGoKart::UpdateLocationFromVelocity(const float& DeltaTime)
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

void AGoKart::Server_SendMove_Implementation(const FGoKartMove & Move)
{
	SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
}

bool AGoKart::Server_SendMove_Validate(const FGoKartMove & Move)
{
	return true;
}

void AGoKart::SimulateMove(const FGoKartMove & Move)
{
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / (Mass > 0.f ? Mass : 1.f);

	Velocity += Acceleration * Move.DeltaTime;

	ApplyRotation(Move.DeltaTime, Move.SteeringThrow);
	UpdateLocationFromVelocity(Move.DeltaTime);
}

FGoKartMove AGoKart::CreateMove(const float & DeltaTime) const
{
	FGoKartMove Move;
	Move.DeltaTime = DeltaTime;
	Move.Throttle = Throttle;
	Move.SteeringThrow = SteeringThrow;
	//Move.Time = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	Move.Time = GetWorld()->GetTimeSeconds();
	
	return Move;
}

void AGoKart::ClearAnacknowledgedMoves(const FGoKartMove& LastMove)
{
	auto PredicateLastStaleMove = [LastMove](const FGoKartMove& lhs)
	{
		return lhs.Time <= LastMove.Time;
	};

	auto RemovedNum = UnacknowledgedMoves.RemoveAll(PredicateLastStaleMove);	
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

void AGoKart::OnRep_ServerState()
{
	UE_LOG(LogTemp, Warning, TEXT("ServerState has been replicated"));
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;

	ClearAnacknowledgedMoves(ServerState.LastMove);

	for (auto Move : UnacknowledgedMoves)
	{
		SimulateMove(Move);
	}
}

void AGoKart::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoKart, ServerState);
}
