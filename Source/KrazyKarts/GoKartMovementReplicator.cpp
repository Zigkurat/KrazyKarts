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
	if (ensure(MovementComponent)) {
		ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
		ClientTimeSinceUpdate = 0;
		ClientStartTransform = GetOwner()->GetActorTransform();
		ClientStartVelocity = MovementComponent->GetVelocity();
	}
}

void UGoKartMovementReplicator::UpdateServerState(const FGoKartMove &Move) {
	ServerState.LastMove = Move;
	ServerState.Velocity = MovementComponent->GetVelocity();
	ServerState.Transform = GetOwner()->GetActorTransform();
}

void UGoKartMovementReplicator::TickClient(float DeltaTime) {
	ClientTimeSinceUpdate += DeltaTime;

	if (ensure(MovementComponent) && ClientTimeBetweenLastUpdates > KINDA_SMALL_NUMBER) {		
		float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;
		FHermiteCubicSpline Spline = CreateSpline();
		
		InterpolateLocation(Spline, LerpRatio);
		InterpolateVelocity(Spline, LerpRatio);
		InterpolateRotation(Spline, LerpRatio);	
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

FHermiteCubicSpline UGoKartMovementReplicator::CreateSpline() {
	FHermiteCubicSpline Spline;
	Spline.StartLocation = ClientStartTransform.GetLocation();
	Spline.TargetLocation = ServerState.Transform.GetLocation();
	Spline.StartDerivative = ClientStartVelocity * ClientTimeBetweenLastUpdates * 100;
	Spline.TargetDerivative = ServerState.Velocity * ClientTimeBetweenLastUpdates * 100;

	return Spline;
}

void UGoKartMovementReplicator::InterpolateLocation(const FHermiteCubicSpline &Spline, float LerpRatio) {
	FVector NewLocation = Spline.InterpolateLocation(LerpRatio);
	GetOwner()->SetActorLocation(NewLocation);
}

void UGoKartMovementReplicator::InterpolateVelocity(const FHermiteCubicSpline &Spline, float LerpRatio) {
	FVector NewDerivative = Spline.InterpolateDerivative(LerpRatio);
	FVector NewVelocity = NewDerivative / (ClientTimeBetweenLastUpdates * 100);
	MovementComponent->SetVelocity(NewVelocity);
}

void UGoKartMovementReplicator::InterpolateRotation(const FHermiteCubicSpline &Spline, float LerpRatio) {
	FQuat StartRotation = ClientStartTransform.GetRotation();
	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);
	GetOwner()->SetActorRotation(NewRotation);
}