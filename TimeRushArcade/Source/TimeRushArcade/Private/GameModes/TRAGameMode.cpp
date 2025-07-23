// TRAGameMode.cpp
#include "TRAGameMode.h"

#include "Characters/TRACharacter.h"
#include "Controllers/TRAPlayerController.h"

ATRAGameMode::ATRAGameMode()
{
	DefaultPawnClass = ATRACharacter::StaticClass();
	PlayerControllerClass = ATRAPlayerController::StaticClass();
}
