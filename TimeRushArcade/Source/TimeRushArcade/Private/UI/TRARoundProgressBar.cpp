// TRARoundProgressBar.cpp
#include "TRARoundProgressBar.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UTRARoundProgressBar::NativeConstruct()
{
	Super::NativeConstruct();

	if (Image_88)
	{
		if (UMaterialInterface* Material = Image_88->GetDynamicMaterial())
		{
			RoundProgressbarInst = Cast<UMaterialInstanceDynamic>(Material);
		}
		else
		{
			RoundProgressbarInst = Image_88->GetDynamicMaterial();
		}
	}
}

void UTRARoundProgressBar::SetProgressPercent(float InPercent)
{
	if (!RoundProgressbarInst && Image_88)
	{
		RoundProgressbarInst = Image_88->GetDynamicMaterial();
	}

	if (RoundProgressbarInst)
	{
		RoundProgressbarInst->SetScalarParameterValue(TEXT("Percent"), FMath::Clamp(InPercent, 0.f, 1.f));
		UE_LOG(LogTemp, Log, TEXT("SetProgressPercent: %.2f"), InPercent);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RoundProgressbarInst is null"));
	}
}