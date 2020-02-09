// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"

USTRUCT() struct FGoKartMove
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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGoKartMovementComponent();

private:
	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10;

	UPROPERTY(EditAnywhere)
	float AirResistanceCoefficient = 16;

	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015;

	FVector Velocity;

	float Throttle;

	float SteeringThrow;

	FGoKartMove LastMove;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SimulateMove(FGoKartMove Move);

	FVector GetVelocity() { return Velocity; };

	void SetVelocity(FVector Val) { Velocity = Val; };

	void SetThrottle(float Val) { Throttle = Val; };

	void SetSteeringThrow(float Val) { SteeringThrow = Val; };

	FGoKartMove GetLastMove() { return LastMove; };

protected:
	virtual void BeginPlay() override;

private:
	FGoKartMove CreateMove(float DeltaTime);
	
	void UpdateLocationFromVelocity(float DeltaTime);

	void ApplyRotation(float DeltaTime, float InSteeringThrow);

	FVector GetAirResistance();

	FVector GetRollingResistance();
};
