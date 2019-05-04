// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementReplicator.h"
#include "UnrealNetwork.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}


// Called when the game starts
void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
	
}


// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		UnacknowledgedMoves.Add(Move);
		Server_SendMove(Move);
		MovementComponent->SimulateMove(Move);
	}
	else if (GetOwnerRole() == ROLE_Authority && GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		Server_SendMove(Move);
	}
	else if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
	}
}

void UGoKartMovementReplicator::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;
	
	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER)
	{
		return;
	}


	float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;
	FHermiteCubicSpline CubicSpline = CreateCubicSpline();

	InterpolateLocation(CubicSpline, LerpRatio);
	InterpolateVelocity(CubicSpline, LerpRatio);
	InterpolateRotation(LerpRatio);
}

void UGoKartMovementReplicator::SetMeshOffsetRoot(USceneComponent * Root)
{
	MeshOffsetRoot = Root;
}

FHermiteCubicSpline UGoKartMovementReplicator::CreateCubicSpline()
{
	FHermiteCubicSpline CubicSpline;
	CubicSpline.TargetLocation = ServerState.Transform.GetLocation();

	CubicSpline.StartLocation = ClientStartTransform.GetLocation();
	CubicSpline.StartDerivative = ClientStartVelocity * ClientTimeBetweenLastUpdates * 100;
	CubicSpline.TargetDerivative = ServerState.Velocity * ClientTimeBetweenLastUpdates * 100;

	return CubicSpline;
}

void UGoKartMovementReplicator::InterpolateLocation(const FHermiteCubicSpline &CubicSpline, const float LerpRatio)
{
	FVector NewLocation = CubicSpline.InterpolateLocation(LerpRatio);

	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldLocation(NewLocation);
	}
}

void UGoKartMovementReplicator::InterpolateVelocity(const FHermiteCubicSpline &CubicSpline, const float LerpRatio)
{
	if (MovementComponent)
	{
		FVector NewDerivative = CubicSpline.InterpolateDerivative(LerpRatio);
		FVector NewVelocity = NewDerivative / (ClientTimeBetweenLastUpdates * 100);
		MovementComponent->SetVelocity(NewVelocity);
	}
}

void UGoKartMovementReplicator::InterpolateRotation(const float LerpRatio)
{
	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat StartRotation = ClientStartTransform.GetRotation();

	FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);

	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldRotation(NewRotation);
	}
}

void UGoKartMovementReplicator::Server_SendMove_Implementation(const FGoKartMove & Move)
{
	if (!MovementComponent)
		return;

	MovementComponent->SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(const FGoKartMove & Move)
{
	return true;
}

void UGoKartMovementReplicator::ClearAnacknowledgedMoves(const FGoKartMove& LastMove)
{
	auto PredicateLastStaleMove = [LastMove](const FGoKartMove& lhs)
	{
		return lhs.Time <= LastMove.Time;
	};

	auto RemovedNum = UnacknowledgedMoves.RemoveAll(PredicateLastStaleMove);
}

void UGoKartMovementReplicator::OnRep_ServerState()
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerState();
		break;
	}
}

void UGoKartMovementReplicator::AutonomousProxy_OnRep_ServerState()
{
	if (!MovementComponent)
		return;

	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAnacknowledgedMoves(ServerState.LastMove);

	for (auto Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UGoKartMovementReplicator::SimulatedProxy_OnRep_ServerState()
{
	if (!MovementComponent)
		return;

	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0.f;

	if (MeshOffsetRoot != nullptr)
	{
		ClientStartTransform.SetLocation(MeshOffsetRoot->GetComponentLocation());
		ClientStartTransform.SetRotation(MeshOffsetRoot->GetComponentQuat());
	}
	
	ClientStartVelocity = MovementComponent->GetVelocity();

	GetOwner()->SetActorTransform(ServerState.Transform);
}

void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
}

