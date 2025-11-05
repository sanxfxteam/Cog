#pragma once

#include "CoreMinimal.h"
#include "CogCommonConfig.h"
#include "CogWindow.h"
#include "CogAIWindow_StateTree.generated.h"

class UStateTreeComponent;
struct FStateTreeStateHandle;
struct FStateTreeExecutionState;
class UCogAIConfig_StateTree;

//--------------------------------------------------------------------------------------------------------------------------
class COGAI_API FCogAIWindow_StateTree : public FCogWindow
{
    typedef FCogWindow Super;

public:

    virtual void Initialize() override;

protected:

    virtual void RenderHelp() override;

    virtual void RenderContent() override;

    virtual void RenderState(UStateTreeComponent& StateTreeComponent, const FStateTreeStateHandle StateHandle, const TArray<FStateTreeStateHandle>& ActiveStates, bool OpenAllChildren);

private:

    ImGuiTextFilter Filter;

    TObjectPtr<UCogAIConfig_StateTree> Config = nullptr;
};

//--------------------------------------------------------------------------------------------------------------------------
UCLASS(Config = Cog)
class UCogAIConfig_StateTree : public UCogCommonConfig
{
    GENERATED_BODY()

public:

    UPROPERTY(Config)
    FVector4f ActiveColor = FVector4f(0.0f, 1.0f, 0.0f, 1.0f);

    UPROPERTY(Config)
    FVector4f InactiveColor = FVector4f(1.0f, 1.0f, 1.0f, 0.2f);

    UPROPERTY(Config)
    FVector4f SelectionColor = FVector4f(1.0f, 0.5f, 0.0f, 1.0f);

    virtual void Reset() override
    {
        Super::Reset();

        ActiveColor = FVector4f(0.0f, 1.0f, 0.0f, 1.0f);
        InactiveColor = FVector4f(1.0f, 1.0f, 1.0f, 0.2f);
        SelectionColor = FVector4f(1.0f, 0.5f, 0.0f, 1.0f);
    }
};
