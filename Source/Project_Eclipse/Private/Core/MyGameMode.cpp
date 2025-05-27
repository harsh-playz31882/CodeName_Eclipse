#include "Core/MyGameMode.h"
#include "HUD/MyHUD.h"

AMyGameMode::AMyGameMode()
{
    HUDClass = AMyHUD::StaticClass();
} 