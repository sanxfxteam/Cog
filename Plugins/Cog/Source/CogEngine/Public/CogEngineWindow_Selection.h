#pragma once

#include "CoreMinimal.h"
#include "CogCommonConfig.h"
#include "GameFramework/Actor.h"
#include "CogWindow.h"
#include "CogEngineWindow_Selection.generated.h"

class IConsoleObject;
class UCogEngineConfig_Selection;
enum class ECogImGuiInputMode : uint8;

//--------------------------------------------------------------------------------------------------------------------------
class COGENGINE_API FCogEngineWindow_Selection : public FCogWindow
{
    typedef FCogWindow Super;

public:

    static FString ToggleSelectionModeCommand;

    virtual void Initialize() override;

    virtual void Shutdown() override;

    const TArray<TSubclassOf<AActor>>& GetActorClasses() const { return ActorClasses; }

    void SetActorClasses(const TArray<TSubclassOf<AActor>>& Value) { ActorClasses = Value; }

    ETraceTypeQuery GetTraceType() const { return TraceType; }

    void SetTraceType(ETraceTypeQuery Value) { TraceType = Value; }

protected:

    virtual void TryReapplySelection() const;

    virtual void ResetConfig() override;

    virtual void PreSaveConfig() override;

    virtual void RenderHelp() override;

    virtual void RenderTick(float DeltaTime) override;

    virtual void RenderContent() override;

    virtual void RenderMainMenuWidget() override;

    virtual bool DrawSelectionCombo();

    virtual void HackWaitInputRelease();

    virtual void SetGlobalSelection(AActor* Value) const;

    virtual void RenderPickButtonTooltip();

    virtual void RenderActorContextMenu(AActor& Actor);

    TSubclassOf<AActor> GetSelectedActorClass() const;

    void TickSelectionMode();

    FVector LastSelectedActorLocation = FVector::ZeroVector;

    bool bIsInputEnabledBeforeEnteringSelectionMode = false;

    int32 WaitInputReleased = 0;

    TArray<TSubclassOf<AActor>> ActorClasses;

    ETraceTypeQuery TraceType = TraceTypeQuery1;

    TObjectPtr<UCogEngineConfig_Selection> Config;

	ImGuiTextFilter Filter;
};

//--------------------------------------------------------------------------------------------------------------------------
UCLASS(Config = Cog)
class UCogEngineConfig_Selection : public UCogCommonConfig
{
    GENERATED_BODY()

public:

    UPROPERTY(Config)
    bool bReapplySelection = true;

    UPROPERTY(Config)
    FString SelectionName;

    UPROPERTY(Config)
    int32 SelectedClassIndex = 0;

    virtual void Reset() override
    {
        Super::Reset();

        bReapplySelection = true;
        SelectionName.Reset();
        SelectedClassIndex = 0;
    }
};