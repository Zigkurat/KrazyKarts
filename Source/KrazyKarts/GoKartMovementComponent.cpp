// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementComponent.h"

UGoKartMovementComponent::UGoKartMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGoKartMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UGoKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGoKartMovementComponent::SimulateMove(FGoKartMove Move)
{
	FVector Force = GetOwner()->GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();
	FVector Acceleration = Force / Mass;

	Velocity += Acceleration * Move.DeltaTime;

	ApplyRotation(Move.DeltaTime, Move.SteeringThrow);
	UpdateLocationFromVelocity(Move.DeltaTime);
}

FGoKartMove UGoKartMovementComponent::CreateMove(float DeltaTime)
{
	FGoKartMove Move;
	Move.Throttle = Throttle;
	Move.SteeringThrow = SteeringThrow;
	Move.DeltaTime = DeltaTime;
	Move.Time = GetWorld()->TimeSeconds;

	return Move;
}

void UGoKartMovementComponent::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector DeltaTranslation = Velocity * 100 * DeltaTime;

	FHitResult HitResult;
	GetOwner()->AddActorWorldOffset(DeltaTranslation, true, &HitResult);
	if (HitResult.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

void UGoKartMovementComponent::ApplyRotation(float DeltaTime, float InSteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = DeltaLocation / MinTurningRadius * InSteeringThrow;
	FQuat DeltaRotation(GetOwner()->GetActorUpVector(), RotationAngle);
	Velocity = DeltaRotation.RotateVector(Velocity);
	GetOwner()->AddActorWorldRotation(DeltaRotation);
}

FVector UGoKartMovementComponent::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * AirResistanceCoefficient;
}

FVector UGoKartMovementComponent::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * NormalForce * RollingResistanceCoefficient;
}