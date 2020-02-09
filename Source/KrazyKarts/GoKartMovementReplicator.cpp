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
		FGoKartMove LastMove = MovementComponent->GetLastMove();
		if (GetOwnerRole() == ROLE_AutonomousProxy)
		{
			UnacknowledgedMoves.Add(LastMove);
			Server_SendMove(LastMove);
		}

		if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
		{
			UpdateServerState(LastMove);
		}

		if (GetOwnerRole() == ROLE_SimulatedProxy)
		{
			TickClient(DeltaTime);
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

		UpdateServerState(Move);
	}
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{
	return true;
}

void UGoKartMovementReplicator::OnRep_ServerState()
{
	if (GetOwnerRole() == ROLE_AutonomousProxy) {
		AuthonomousProxy_OnRep_ServerState();
	}
	if (GetOwnerRole() == ROLE_SimulatedProxy) {
		SimulatedProxy_OnRep_ServerState();
	}
}

void UGoKartMovementReplicator::AuthonomousProxy_OnRep_ServerState() {
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

void UGoKartMovementReplicator::SimulatedProxy_OnRep_ServerState() {
	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0;
	ClientStartTransform = GetOwner()->GetActorTransform();
}

void UGoKartMovementReplicator::UpdateServerState(const FGoKartMove &Move) {
	ServerState.LastMove = Move;
	ServerState.Velocity = MovementComponent->GetVelocity();
	ServerState.Transform = GetOwner()->GetActorTransform();
}

void UGoKartMovementReplicator::TickClient(float DeltaTime) {
	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdates > KINDA_SMALL_NUMBER) {
		float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;

		FVector StartLocation = ClientStartTransform.GetLocation();
		FVector TargetLocation = ServerState.Transform.GetLocation();
		FVector NewLocation = FMath::LerpStable(StartLocation, TargetLocation, LerpRatio);
		GetOwner()->SetActorLocation(NewLocation);

		FQuat StartRotation = ClientStartTransform.GetRotation();
		FQuat TargetRotation = ServerState.Transform.GetRotation();
		FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);
		GetOwner()->SetActorRotation(NewRotation);
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
