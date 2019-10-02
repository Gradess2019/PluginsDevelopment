// Copyright 2017-2019 MARTA. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/TimelineComponent.h"
#include "VRPackStructs.h"
#include "Picture.h"
#include "PaintComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRPACK_API UPaintComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	UPaintComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Paint component")
	void StartDrawing();
	
	UFUNCTION(BlueprintCallable, Category = "Paint component")
	void StopDrawing();

	UFUNCTION(BlueprintCallable, Category = "Paint component")
	void FinishDrawing();

private:

	UPROPERTY(EditDefaultsOnly, Category = "Paint component")
	FTimelineSettings TimelineSettings;

	UPROPERTY(EditDefaultsOnly, Category = "Paint component")
	float SliceSize;

	UPROPERTY(EditDefaultsOnly, Category = "Paint component")
	float AllowableAngle;
	
	UPROPERTY(EditDefaultsOnly, Category = "Paint component")
	UStaticMesh* SplineMesh;

	TWeakObjectPtr<UTimelineComponent> DrawingTimeline;

	TWeakObjectPtr<APicture> CurrentPicture;

	void InitializeComponent() override;
	void InitializeDrawingTimeline();

	UFUNCTION()
	void OnUpdatePaint();
};
