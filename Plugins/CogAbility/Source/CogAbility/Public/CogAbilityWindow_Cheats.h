#pragma once

#include "CoreMinimal.h"
#include "CogWindow.h"
#include "CogWindowConfig.h"
#include "CogAbilityWindow_Cheats.generated.h"

class AActor;
class UCogAbilityConfig_Alignment;
class UCogAbilityConfig_Cheats;
class UCogAbilityDataAsset;
struct FCogAbilityCheat;

//--------------------------------------------------------------------------------------------------------------------------
class COGABILITY_API FCogAbilityWindow_Cheats : public FCogWindow
{
    typedef FCogWindow Super;

public:

    virtual void Initialize() override;

protected:

    virtual void GameTick(float DeltaTime);

    virtual void ResetConfig() override;

    virtual void RenderHelp() override;

    virtual void RenderContent() override;

    virtual void TryReapplyCheats();

    virtual bool AddCheat(AActor* ControlledActor, AActor* TargetActor, const FCogAbilityCheat& CheatEffect, bool IsPersistent);

    virtual void RequestCheat(AActor* ControlledActor, AActor* TargetActor, const FCogAbilityCheat& CheatEffect);

    TObjectPtr<const UCogAbilityDataAsset> Asset = nullptr;

    TObjectPtr<UCogAbilityConfig_Cheats> Config = nullptr;

    TObjectPtr<UCogAbilityConfig_Alignment> AlignmentConfig = nullptr;

    bool bHasReappliedCheats = false;
};

//--------------------------------------------------------------------------------------------------------------------------
UCLASS(Config = Cog)
class UCogAbilityConfig_Cheats : public UCogWindowConfig
{
    GENERATED_BODY()

public:

    UPROPERTY(Config)
    bool bReapplyCheatsBetweenPlays = true;

    UPROPERTY(Config)
    bool bReapplyCheatsBetweenLaunches = true;

    UPROPERTY(Config)
    TArray<FString> AppliedCheats;

    virtual void Reset() override
    {
        Super::Reset();

        bReapplyCheatsBetweenPlays = true;
        bReapplyCheatsBetweenLaunches = true;
    }
};
