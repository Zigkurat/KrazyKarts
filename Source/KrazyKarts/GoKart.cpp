// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();
}

void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();
	FVector Acceleration = Force / Mass;

	

	Velocity += Acceleration * DeltaTime;
	
	ApplyRotation(DeltaTime);
	UpdateLocationFromVelocity(DeltaTime);
}

void AGoKart::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::Server_MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::Server_MoveRight);
}

void AGoKart::Server_MoveForward_Implementation(float Value)
{
	Throttle = Value;
}

bool AGoKart::Server_MoveForward_Validate(float Value) {
	return FMath::Abs(Value) <= 1;
}

void AGoKart::Server_MoveRight_Implementation(float Value) {
	SteeringThrow = Value;
}

bool AGoKart::Server_MoveRight_Validate(float Value) {
	return FMath::Abs(Value) <= 1;
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime) {
	FVector DeltaTranslation = Velocity * 100 * DeltaTime;

	FHitResult HitResult;
	AddActorWorldOffset(DeltaTranslation, true, &HitResult);
	if (HitResult.IsValidBlockingHit()) {
		Velocity = FVector::ZeroVector;
	}
}

void AGoKart::ApplyRotation(float DeltaTime) {
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = DeltaLocation / MinTurningRadius * SteeringThrow;
	FQuat DeltaRotation(GetActorUpVector(), RotationAngle);
	Velocity = DeltaRotation.RotateVector(Velocity);
	AddActorWorldRotation(DeltaRotation);
}

FVector AGoKart::GetAirResistance() {
	return - Velocity.GetSafeNormal() * Velocity.SizeSquared() * AirResistanceCoefficient; 
}

FVector AGoKart::GetRollingResistance() {
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = Mass * AccelerationDueToGravity;
	return - Velocity.GetSafeNormal() * NormalForce * RollingResistanceCoefficient;
}