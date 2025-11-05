#include "CogAIWindow_StateTree.h"

#include "AIController.h"
#include "CogAIModule.h"
#include "CogImguiHelper.h"
#include "CogWindowWidgets.h"
#include "GameFramework/Pawn.h"
#include "imgui_internal.h"
#include "StateTreeComponent.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeTypes.h"

//--------------------------------------------------------------------------------------------------------------------------
void FCogAIWindow_StateTree::Initialize()
{
    Super::Initialize();

    bHasMenu = true;

    Config = GetConfig<UCogAIConfig_StateTree>();
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogAIWindow_StateTree::RenderHelp()
{
    ImGui::Text(
        "This window displays the state tree of the selected actor. "
        "Active states are shown in green. "
    );
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogAIWindow_StateTree::RenderContent()
{
    Super::RenderContent();

    if (Config == nullptr)
    {
        ImGui::TextDisabled("Invalid Config");
        return;
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            ImGui::ColorEdit4("Active Color", (float*)&Config->ActiveColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreviewHalf);
            ImGui::ColorEdit4("Inactive Color", (float*)&Config->InactiveColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreviewHalf);
            ImGui::ColorEdit4("Selection Color", (float*)&Config->SelectionColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreviewHalf);
            ImGui::EndMenu();
        }

        FCogWindowWidgets::SearchBar("##Filter", Filter);

        ImGui::EndMenuBar();
    }

    AActor* Selection = GetSelection();
    if (Selection == nullptr)
    {
        ImGui::TextDisabled("Invalid Selection");
        return;
    }

    UStateTreeComponent* StateTreeComponent = Selection->FindComponentByClass<UStateTreeComponent>();
    if (StateTreeComponent == nullptr)
    {
        ImGui::TextDisabled("Selection has no StateTreeComponent");
        return;
    }

    if (!StateTreeComponent->IsTreeStarted())
    {
        ImGui::TextDisabled("State Tree is not started");
        return;
    }

    const UStateTree* StateTree = StateTreeComponent->GetStateTree();
    if (StateTree == nullptr)
    {
        ImGui::TextDisabled("Selection has no StateTree asset");
        return;
    }

    // Get active states
    TArray<FStateTreeStateHandle> ActiveStates;
    const FStateTreeExecutionState* ExecutionState = StateTreeComponent->GetExecutionState();
    if (ExecutionState != nullptr)
    {
        for (int32 i = 0; i < ExecutionState->ActiveStates.Num(); ++i)
        {
            if (ExecutionState->ActiveStates[i].IsValid())
            {
                ActiveStates.Add(ExecutionState->ActiveStates[i]);
            }
        }
    }

    if (ImGui::CollapsingHeader(TCHAR_TO_ANSI(*GetNameSafe(StateTree)), nullptr, ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Start rendering from root state
        FStateTreeStateHandle RootState = FStateTreeStateHandle(0);
        RenderState(*StateTreeComponent, RootState, ActiveStates, false);
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogAIWindow_StateTree::RenderState(UStateTreeComponent& StateTreeComponent, const FStateTreeStateHandle StateHandle, const TArray<FStateTreeStateHandle>& ActiveStates, bool OpenAllChildren)
{
    if (!StateHandle.IsValid())
    {
        return;
    }

    const UStateTree* StateTree = StateTreeComponent.GetStateTree();
    if (StateTree == nullptr)
    {
        return;
    }

    // Get state information
    FStateTreeExecutionContext ExecutionContext(StateTreeComponent.GetOwner(), *StateTree, StateTreeComponent);
    const FCompactStateTreeState* State = ExecutionContext.GetStateFromHandle(StateHandle);
    if (State == nullptr)
    {
        return;
    }

    // Get state name
    FString StateName = State->Name.ToString();
    const auto StateNameAnsi = StringCast<ANSICHAR>(*StateName);
    const bool ShowState = Filter.PassFilter(StateNameAnsi.Get());

    // Check if this state is active
    const bool IsActive = ActiveStates.Contains(StateHandle);

    // Check if this state has the current selection (last active state in the chain)
    const bool IsSelection = ActiveStates.Num() > 0 && ActiveStates.Last() == StateHandle;

    bool OpenChildren = false;

    if (ShowState)
    {
        ImGui::PushID(StateHandle.Index);

        if (OpenAllChildren)
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        }
        else
        {
            ImGui::SetNextItemOpen(IsActive, ImGuiCond_Once);
        }

        //------------------------
        // TreeNode
        //------------------------
        const bool HasChildren = State->ChildrenEnd > State->ChildrenBegin;
        if (HasChildren && Filter.IsActive() == false)
        {
            OpenChildren = ImGui::TreeNodeEx("##State", ImGuiSelectableFlags_AllowOverlap | ImGuiTreeNodeFlags_SpanFullWidth);
        }
        else
        {
            ImGui::TreeNodeEx("##State", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiSelectableFlags_AllowOverlap | ImGuiTreeNodeFlags_SpanFullWidth);
        }

        const bool IsControlDown = ImGui::GetCurrentContext()->IO.KeyCtrl;
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && IsControlDown)
        {
            OpenAllChildren = true;
        }

        //------------------------
        // ContextMenu
        //------------------------
        if (ImGui::BeginPopupContextItem())
        {
            ImGui::EndPopup();
        }

        //------------------------
        // Tooltip
        //------------------------
        if (FCogWindowWidgets::BeginItemTableTooltip())
        {
            if (ImGui::BeginTable("StateInfo", 2, ImGuiTableFlags_Borders))
            {
                ImGui::TableSetupColumn("Property");
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                const ImVec4 TextColor(1.0f, 1.0f, 1.0f, 0.5f);

                //------------------------
                // Name
                //------------------------
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextColored(TextColor, "Name");
                ImGui::TableNextColumn();
                ImGui::Text("%s", StateNameAnsi.Get());

                //------------------------
                // State Type
                //------------------------
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextColored(TextColor, "Type");
                ImGui::TableNextColumn();

                FString StateType = "State";
                if (State->Type == EStateTreeStateType::Linked)
                {
                    StateType = "Linked";
                }
                else if (State->Type == EStateTreeStateType::LinkedAsset)
                {
                    StateType = "Linked Asset";
                }
                else if (State->Type == EStateTreeStateType::Subtree)
                {
                    StateType = "Subtree";
                }
                else if (State->Type == EStateTreeStateType::Group)
                {
                    StateType = "Group";
                }
                ImGui::Text("%s", TCHAR_TO_ANSI(*StateType));

                //------------------------
                // State Index
                //------------------------
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextColored(TextColor, "Index");
                ImGui::TableNextColumn();
                ImGui::Text("%d", StateHandle.Index);

                //------------------------
                // Active Status
                //------------------------
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextColored(TextColor, "Active");
                ImGui::TableNextColumn();
                ImGui::Text("%s", IsActive ? "Yes" : "No");

                //------------------------
                // Tasks Count
                //------------------------
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextColored(TextColor, "Tasks");
                ImGui::TableNextColumn();
                ImGui::Text("%d", State->TasksNum);

                //------------------------
                // Transitions Count
                //------------------------
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextColored(TextColor, "Transitions");
                ImGui::TableNextColumn();
                ImGui::Text("%d", State->TransitionsNum);

                ImGui::EndTable();
            }

            FCogWindowWidgets::EndItemTableTooltip();
        }

        //------------------------
        // Checkbox
        //------------------------
        ImGui::SameLine();
        FCogWindowWidgets::PushStyleCompact();
        if (IsActive == false)
        {
            ImGui::BeginDisabled();
        }
        bool DrawActive = IsActive;
        ImGui::Checkbox("##Active", &DrawActive);

        if (IsActive == false)
        {
            ImGui::EndDisabled();
        }
        FCogWindowWidgets::PopStyleCompact();

        //------------------------
        // Name
        //------------------------
        ImGui::SameLine();
        ImVec4 NameColor;
        if (IsSelection)
        {
            NameColor = FCogImguiHelper::ToImVec4(Config->SelectionColor);
        }
        else if (IsActive)
        {
            NameColor = FCogImguiHelper::ToImVec4(Config->ActiveColor);
        }
        else
        {
            NameColor = FCogImguiHelper::ToImVec4(Config->InactiveColor);
        }
        ImGui::TextColored(NameColor, "%s", StateNameAnsi.Get());
    }

    //------------------------
    // Children
    //------------------------
    if (OpenChildren || Filter.IsActive())
    {
        if (State->ChildrenEnd > State->ChildrenBegin)
        {
            for (uint16 ChildIndex = State->ChildrenBegin; ChildIndex < State->ChildrenEnd; ++ChildIndex)
            {
                FStateTreeStateHandle ChildHandle = FStateTreeStateHandle((uint16)ChildIndex);
                RenderState(StateTreeComponent, ChildHandle, ActiveStates, OpenAllChildren);
            }
        }
    }

    if (ShowState)
    {
        if (OpenChildren)
        {
            ImGui::TreePop();
        }

        ImGui::PopID();
    }
}
