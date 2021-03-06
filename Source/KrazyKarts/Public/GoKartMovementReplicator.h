// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.h"
#include "GoKartMovementReplicator.generated.h"


USTRUCT()
struct FGoKartState
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		FGoKartMove LastMove;

	UPROPERTY()
		FTransform Transform;

	UPROPERTY()
		FVector Velocity;
};

struct FHermiteCubicSpline
{
	FVector StartLocation;
	FVector StartDerivative;
	FVector TargetLocation;
	FVector TargetDerivative;

	FVector InterpolateLocation(float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}

	FVector InterpolateDerivative(float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}

};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementReplicator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementReplicator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ClearAnacknowledgedMoves(const FGoKartMove& LastMove);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(const FGoKartMove & Move);

	void ClientTick(float DeltaTime);

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoKartState ServerState;

	UFUNCTION()
	virtual void OnRep_ServerState();

	void AutonomousProxy_OnRep_ServerState();
	void SimulatedProxy_OnRep_ServerState();

	TArray<FGoKartMove> UnacknowledgedMoves;

	float ClientTimeSinceUpdate;
	float ClientTimeBetweenLastUpdates;

	FTransform ClientStartTransform;
	FVector ClientStartVelocity;

	float ClientSimulatedTime;

	UPROPERTY()
	UGoKartMovementComponent* MovementComponent;

	UPROPERTY()
	USceneComponent* MeshOffsetRoot;

	UFUNCTION(BlueprintCallable)
	void SetMeshOffsetRoot(USceneComponent* Root);

	FHermiteCubicSpline CreateCubicSpline();
	void InterpolateLocation(const FHermiteCubicSpline &CubicSpline, const float LerpRatio);
	void InterpolateVelocity(const FHermiteCubicSpline &CubicSpline, const float LerpRatio);
	void InterpolateRotation(const float LerpRatio);
		
};
