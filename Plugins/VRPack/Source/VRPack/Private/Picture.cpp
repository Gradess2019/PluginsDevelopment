// Copyright 2017-2019 MARTA. All Rights Reserved.

#include "Picture.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

APicture::APicture(const FObjectInitializer& ObjectInitializer)
{
	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "RootSceneComponent");
}

void APicture::InitializePicture(FPictureSettings PictureSettings)
{
	this->PictureSettings = PictureSettings;
}

void APicture::FinishFollowing()
{
	LastDrawnSlice = nullptr;
	CurrentSlice = nullptr;
}

void APicture::Follow(FVector PointLocation)
{
	if (!IsAllowableSize() || !IsAllowableAngle())
	{
		CalculateSplineTangentAndPositions(PointLocation);
		CreateNewMesh(PointLocation);
	}
	CalculateSplineTangentAndPositions(PointLocation);
}

bool APicture::IsAllowableSize() const
{
	return (CurrentSlice->GetEndPosition() - CurrentSlice->GetStartPosition()).Size() < PictureSettings.SliceSize;
}

bool APicture::IsAllowableAngle() const
{
	if (!LastDrawnSlice.IsValid()) return false;
	if (CurrentSlice->GetEndPosition().Equals(FVector::ZeroVector, PictureSettings.SliceAngleTolerance)) return true;

	return PictureSettings.AllowableAngle > GetAngle();
}

float APicture::GetAngle() const
{
	const FVector LastSplineDirection = GetNormalizedDirection(LastDrawnSlice.Get());
	const FVector CurrentSplineDirection = GetNormalizedDirection(CurrentSlice.Get());

	const float DotProduct = UKismetMathLibrary::Dot_VectorVector(CurrentSplineDirection, LastSplineDirection);
	return UKismetMathLibrary::DegAcos(DotProduct);
}

FVector APicture::GetNormalizedDirection(const USplineMeshComponent* SplineMesh) const
{
	return UKismetMathLibrary::Normal(SplineMesh->GetEndPosition() - SplineMesh->GetStartPosition());
}

void APicture::CalculateSplineTangentAndPositions(FVector PointLocation) const
{
	const FVector EndPosition = UKismetMathLibrary::InverseTransformLocation(
		CurrentSlice->GetComponentTransform(), PointLocation
	);
	
	const FVector EndTangent = CurrentSlice->GetEndPosition() - CurrentSlice->GetStartPosition();

	if (!LastDrawnSlice.IsValid())
	{
		CurrentSlice->SetStartTangent(EndTangent);
	}

	CurrentSlice->SetEndTangent(EndTangent);
	CurrentSlice->SetEndPosition(EndPosition);
}

void APicture::CreateNewMesh(FVector PointLocation)
{
	const FTransform ParentTransform = GetParentTransform();
	const FVector LocalPointPositition = UKismetMathLibrary::InverseTransformLocation(ParentTransform, PointLocation);

	FSplineMeshInitializer Initializer;
	FVector RelativeLocation;

	if (CurrentSlice.IsValid())
	{
		RelativeLocation = CurrentSlice->GetEndPosition() + CurrentSlice->RelativeLocation;

		Initializer = FSplineMeshInitializer(
			FVector::ZeroVector,
			CurrentSlice->GetEndTangent(),
			LocalPointPositition,
			LocalPointPositition
		);
		
		LastDrawnSlice = CurrentSlice;
	}
	else
	{
		RelativeLocation = LocalPointPositition;
	}

	CreateSplineMeshComponent(RelativeLocation);
	SetStartAndEnd(Initializer);
	UE_LOG(LogTemp, Warning, TEXT("New mesh was created"));
}

FTransform APicture::GetParentTransform() const
{
	return CurrentSlice.IsValid()
		       ? CurrentSlice->GetComponentTransform()
		       : GetActorTransform();
}

void APicture::CreateSplineMeshComponent(FVector RelativeLocation)
{
	CurrentSlice = NewObject<USplineMeshComponent>(this);
	CurrentSlice->SetMobility(EComponentMobility::Movable);
	CurrentSlice->RegisterComponent();
	CurrentSlice->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	CurrentSlice->SetRelativeLocation(RelativeLocation);
	CurrentSlice->SetStaticMesh(PictureSettings.StaticMesh);
}

void APicture::SetStartAndEnd(const FSplineMeshInitializer& Initializer) const
{
	CurrentSlice->SetStartAndEnd(
		Initializer.StartPos, 
		Initializer.StartTangent, 
		Initializer.EndPos,
		Initializer.EndTangent
	);
}
