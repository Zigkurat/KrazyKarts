// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) {
		NetUpdateFrequency = 1;
	}
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

	if (HasAuthority()) {
		ReplicatedTransform = GetActorTransform();
	}
}

void AGoKart::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoKart, ReplicatedTransform);
}

void AGoKart::Server_MoveForward_Implementation(float Value)
{
	Throttle = Value;
}

bool AGoKart::Server_MoveForward_Validate(float Value)
{
	return FMath::Abs(Value) <= 1;
}

void AGoKart::Server_MoveRight_Implementation(float Value)
{
	SteeringThrow = Value;
}

bool AGoKart::Server_MoveRight_Validate(float Value)
{
	return FMath::Abs(Value) <= 1;
}

void AGoKart::OnRep_ReplicatedTransform() {
	SetActorTransform(ReplicatedTransform);
}

void AGoKart::MoveForward(float Value)
{
	Throttle = Value;
	Server_MoveForward(Value);
}

void AGoKart::MoveRight(float Value)
{
	SteeringThrow = Value;
	Server_MoveRight(Value);
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector DeltaTranslation = Velocity * 100 * DeltaTime;

	FHitResult HitResult;
	AddActorWorldOffset(DeltaTranslation, true, &HitResult);
	if (HitResult.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

void AGoKart::ApplyRotation(float DeltaTime)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = DeltaLocation / MinTurningRadius * SteeringThrow;
	FQuat DeltaRotation(GetActorUpVector(), RotationAngle);
	Velocity = DeltaRotation.RotateVector(Velocity);
	AddActorWorldRotation(DeltaRotation);
}

FVector AGoKart::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * AirResistanceCoefficient;
}

FVector AGoKart::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * NormalForce * RollingResistanceCoefficient;
}