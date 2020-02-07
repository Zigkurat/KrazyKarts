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

	FVector DeltaTranslation = Velocity * 100 * DeltaTime;
	AddActorWorldOffset(DeltaTranslation);
}

void AGoKart::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
}

void AGoKart::MoveForward(float Value)
{
	Velocity = GetActorForwardVector() * 20 * Value;
}