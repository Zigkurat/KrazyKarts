// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("MovementComponent"));
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}

void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ensure(MovementComponent))
	{
		if (Role == ROLE_AutonomousProxy)
		{
			FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
			UnacknowledgedMoves.Add(Move);
			MovementComponent->SimulateMove(Move);
			Server_SendMove(Move);
		}

		if (Role == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
		{
			FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
			Server_SendMove(Move);
		}

		if (Role == ROLE_SimulatedProxy)
		{
			MovementComponent->SimulateMove(ServerState.LastMove);
		}
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

	DOREPLIFETIME(AGoKart, ServerState);
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (ensure(MovementComponent))
	{
		MovementComponent->SimulateMove(Move);

		ServerState.LastMove = Move;
		ServerState.Velocity = MovementComponent->GetVelocity();
		ServerState.Transform = GetActorTransform();
	}
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
	return true;
}

void AGoKart::OnRep_ServerState()
{
	if (ensure(MovementComponent))
	{
		SetActorTransform(ServerState.Transform);
		MovementComponent->SetVelocity(ServerState.Velocity);

		ClearAcknowledgedMoves(ServerState.LastMove);

		for (const FGoKartMove &Move : UnacknowledgedMoves)
		{
			MovementComponent->SimulateMove(Move);
		}
	}
}

void AGoKart::ClearAcknowledgedMoves(FGoKartMove LastMove)
{
	TArray<FGoKartMove> NewMoves;
	for (const FGoKartMove &Move : UnacknowledgedMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

void AGoKart::MoveForward(float Value)
{
	if (ensure(MovementComponent)) {
		MovementComponent->SetThrottle(Value);
	}
}

void AGoKart::MoveRight(float Value)
{
	if (ensure(MovementComponent)) {
		MovementComponent->SetSteeringThrow(Value);
	}
}
