// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"


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

	bool isValid() const
	{
		return (FMath::Abs(Throttle) <= 1) && (FMath::Abs(SteeringThrow) <= 1);
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SimulateMove(const FGoKartMove& Move);

	FGoKartMove CreateMove(const float& DeltaTime) const;

	FVector GetVelocity() { return Velocity; };
	void SetVelocity(const FVector& Val) { Velocity = Val; };

	void SetThrottle(const float& Val) { Throttle = Val; };
	void SetSteeringThrow(const float& Val) { SteeringThrow = Val; };

private:

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

	FVector Velocity;
	float Throttle;
	float SteeringThrow;
};
