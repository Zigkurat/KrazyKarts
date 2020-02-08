// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "Components/InputComponent.h"

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
	Force += GetResistance();
	FVector Acceleration = Force / Mass;

	Velocity += Acceleration * DeltaTime;
	
	ApplyRotation(DeltaTime);
	UpdateLocationFromVelocity(DeltaTime);
}

void AGoKart::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::MoveForward(float Value)
{
	Throttle = Value;
}

void AGoKart::MoveRight(float Value) {
	SteeringThrow = Value;
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
	float RotationAngle = MaxDegreesPerSecond * DeltaTime * SteeringThrow;
	FQuat DeltaRotation(GetActorUpVector(), FMath::DegreesToRadians(RotationAngle));
	Velocity = DeltaRotation.RotateVector(Velocity);
	AddActorWorldRotation(DeltaRotation);
}

FVector AGoKart::GetResistance() {
	return - Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient; 
}