// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementReplicator.h"
#include "Net/UnrealNetwork.h"

UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}

void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
}

void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ensure(MovementComponent))
	{
		if (GetOwnerRole() == ROLE_AutonomousProxy)
		{
			FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
			UnacknowledgedMoves.Add(Move);
			MovementComponent->SimulateMove(Move);
			Server_SendMove(Move);
		}

		if (GetOwnerRole() == ROLE_Authority && GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
		{
			FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
			Server_SendMove(Move);
		}

		if (GetOwnerRole() == ROLE_SimulatedProxy)
		{
			MovementComponent->SimulateMove(ServerState.LastMove);
		}
	}
}

void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
}

void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (ensure(MovementComponent))
	{
		MovementComponent->SimulateMove(Move);

		ServerState.LastMove = Move;
		ServerState.Velocity = MovementComponent->GetVelocity();
		ServerState.Transform = GetOwner()->GetActorTransform();
	}
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{
	return true;
}

void UGoKartMovementReplicator::OnRep_ServerState()
{
	if (ensure(MovementComponent))
	{
		GetOwner()->SetActorTransform(ServerState.Transform);
		MovementComponent->SetVelocity(ServerState.Velocity);

		ClearAcknowledgedMoves(ServerState.LastMove);

		for (const FGoKartMove &Move : UnacknowledgedMoves)
		{
			MovementComponent->SimulateMove(Move);
		}
	}
}

void UGoKartMovementReplicator::ClearAcknowledgedMoves(FGoKartMove LastMove)
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
