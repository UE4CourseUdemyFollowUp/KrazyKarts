// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;
};

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


UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;	

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	void SimulateMove(const FGoKartMove& Move);

	FGoKartMove CreateMove(const float& DeltaTime) const;

	void ClearAnacknowledgedMoves(const FGoKartMove& LastMove);

	FVector GetAirResistance();
	FVector GetRollingResistance();

	void UpdateLocationFromVelocity(const float& DeltaTime);
	void ApplyRotation(const float& DeltaTime, const float& SteeringThrow);

	UPROPERTY(EditAnywhere)
	float Mass;

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce;

	UPROPERTY(EditAnywhere)
	float MinTurningRadius;

	UPROPERTY(EditAnywhere)
	float DragCoefficient;

	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient;

	void MoveForward(float Val);

	void MoveRight(float Val);

	UFUNCTION( Server, Reliable, WithValidation)
	void Server_SendMove(const FGoKartMove & Move);

	UPROPERTY(ReplicatedUsing=OnRep_ServerState)
	FGoKartState ServerState;

	UFUNCTION()
	virtual void OnRep_ServerState();

	TArray<FGoKartMove> UnacknowledgedMoves;

	FVector Velocity;
	float Throttle;
	float SteeringThrow;
};
