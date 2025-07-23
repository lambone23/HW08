// TRARoundProgressBar.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TRARoundProgressBar.generated.h"

class UImage;
class UMaterialInstanceDynamic;

UCLASS()
class TIMERUSHARCADE_API UTRARoundProgressBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetProgressPercent(float InPercent);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UImage* Image_88;

private:
	UPROPERTY()
	UMaterialInstanceDynamic* RoundProgressbarInst;
};
