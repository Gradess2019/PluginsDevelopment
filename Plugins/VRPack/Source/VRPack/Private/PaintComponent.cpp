// Copyright 2017-2019 MARTA. All Rights Reserved.

#include "PaintComponent.h"
#include "Components/TimelineComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

UPaintComponent::UPaintComponent(const FObjectInitializer& ObjectInitializer)
{
	bWantsInitializeComponent = true;
	TimelineSettings.Loop = true;
	SliceSize = 10.f;
}

void UPaintComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	InitializeDrawingTimeline();

	BrushSceneComponent = NewObject<USceneComponent>(this);
	BrushSceneComponent->RegisterComponent();
	BrushSceneComponent->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	BrushSceneComponent->SetRelativeLocation(BrushSceneComponent->RelativeLocation + BrushOffset);
}

void UPaintComponent::InitializeDrawingTimeline()
{
	DrawingTimeline = NewObject<UTimelineComponent>(this, "DrawingTimeline");

	DrawingTimeline->SetLooping(TimelineSettings.Loop);
	DrawingTimeline->SetIgnoreTimeDilation(TimelineSettings.IgnoreTimeDilation);

	DrawingTimeline->SetTimelineLength(TimelineSettings.Length);
	DrawingTimeline->SetPlayRate(TimelineSettings.PlayRate);
	
	DrawingTimeline->SetTimelineLengthMode(TimelineSettings.Mode);

	FOnTimelineEvent TimelineEvent;
	TimelineEvent.BindUFunction(this, "OnUpdatePaint");
	DrawingTimeline->SetTimelinePostUpdateFunc(TimelineEvent);

	DrawingTimeline->RegisterComponent();
}

void UPaintComponent::OnUpdatePaint()
{
	CurrentPicture->Follow(this->GetComponentLocation());
}

void UPaintComponent::StartDrawing()
{
#if !UE_BUILD_SHIPPING
	if (!DrawingTimeline.IsValid()) checkNoEntry();
#endif

	if (!CurrentPicture.IsValid())
	{
		FActorSpawnParameters Parameters;
		Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Parameters.Owner = GetOwner();
		CurrentPicture = GetWorld()->SpawnActor<APicture>(Parameters);
		CurrentPicture->SetActorTransform(FTransform());
		CurrentPicture->InitializePicture(SliceSize, SplineMesh);
	}

	CurrentPicture->CreateNewMesh(this->GetComponentLocation());

	
#if !UE_BUILD_SHIPPING
	if (!CurrentPicture.IsValid()) checkNoEntry();
#endif
	
	DrawingTimeline->PlayFromStart();
}

void UPaintComponent::StopDrawing()
{
#if !UE_BUILD_SHIPPING
	if (!DrawingTimeline.IsValid()) checkNoEntry();
#endif

	DrawingTimeline->Stop();
}

void UPaintComponent::FinishDrawing()
{
	DrawingTimeline->Stop();
	CurrentPicture = nullptr;
}
