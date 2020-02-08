// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	AGoKart();

private:
	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	UPROPERTY(EditAnywhere)
	float MaxDegreesPerSecond = 90;

	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	FVector Velocity;

	float Throttle;

	float SteeringThrow;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

private:
	void MoveForward(float Value);

	void MoveRight(float Value);

	void UpdateLocationFromVelocity(float DeltaTime);

	void ApplyRotation(float DeltaTime);

	FVector GetResistance();
};
