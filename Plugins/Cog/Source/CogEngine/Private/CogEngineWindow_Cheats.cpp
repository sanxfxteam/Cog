#include "CogEngineWindow_Cheats.h"

#include "CogEngineDataAsset.h"
#include "CogEngineReplicator.h"
#include "CogCommonAllegianceActorInterface.h"
#include "CogImguiHelper.h"
#include "CogWindowConsoleCommandManager.h"
#include "CogWindowWidgets.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "imgui.h"
#include "imgui_internal.h"

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Cheats::RenderHelp()
{
    ImGui::Text(
        "This window can be used to apply cheats to the selected actor (by default). "
        "The cheats can be configured in the '%s' data asset. "
        "When clicking a cheat button, press:\n"
        "   [CTRL]  to apply the cheat to controlled actor\n"
        "   [ALT]   to apply the cheat to the allies of the selected actor\n"
        "   [SHIFT] to apply the cheat to the enemies of the selected actor\n"
        , TCHAR_TO_ANSI(*GetNameSafe(Asset.Get()))
    );
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Cheats::Initialize()
{
    Super::Initialize();

    bHasMenu = true;

    Asset = GetAsset<UCogEngineDataAsset>();
    Config = GetConfig<UCogEngineConfig_Cheats>();

    FCogWindowConsoleCommandManager::RegisterWorldConsoleCommand(
        TEXT("Cog.Cheat"),
        TEXT("Apply a cheat to the selection. Cog.Cheat <CheatName> -Allies -Enemies -Controlled"),
        GetWorld(),
        FCogWindowConsoleCommandDelegate::CreateLambda([this](const TArray<FString>& InArgs, UWorld* InWorld)
            {
                if (InArgs.Num() > 0)
                {
                    if (const FCogEngineCheat* cheat = FindCheatByName(InArgs[0], false))
                    {
                        const bool ApplyToEnemies = InArgs.Contains("-Enemies");
                        const bool ApplyToAllies = InArgs.Contains("-Allies");
                        const bool ApplyToControlled = InArgs.Contains("-Controlled");

                        RequestCheat(GetLocalPlayerPawn(), GetSelection(), *cheat, ApplyToEnemies, ApplyToAllies, ApplyToControlled);
                    }
                    else
                    {
                        UE_LOG(LogCogImGui, Warning, TEXT("Cog.Cheat %s | Cheat not found"), *InArgs[0]);
                    }
                }
            }));


    if (Asset == nullptr)
    { return; }

    for (const FCogEngineCheatCategory& CheatCategory : Asset->CheatCategories)
    {
        for (const FCogEngineCheat& Cheat : CheatCategory.PersistentEffects)
        {
            UpdateCheatColor(Cheat);
        }

        for (const FCogEngineCheat& Cheat : CheatCategory.InstantEffects)
        {
            UpdateCheatColor(Cheat);
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Cheats::UpdateCheatColor(const FCogEngineCheat& Cheat) const
{
    FLinearColor CustomColor;
    if (Cheat.Execution != nullptr && Cheat.Execution->GetColor(*this, CustomColor))
    {
        Cheat.CustomColor = CustomColor;
    }
    else
    {
        Cheat.CustomColor = Cheat.Color;
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Cheats::ResetConfig()
{
    Super::ResetConfig();

    Config->Reset();
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Cheats::GameTick(float DeltaTime)
{
    Super::GameTick(DeltaTime);

    TryReapplyCheats();
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Cheats::TryReapplyCheats()
{
    if (Config == nullptr)
    { return; }
    
    if (bHasReappliedCheats)
    { return; }

    if (Config->bReapplyCheatsBetweenPlays == false)
    { return; }

    static int32 IsFirstLaunch = true;
    if (IsFirstLaunch && Config->bReapplyCheatsBetweenLaunches == false)
    { return; }
    IsFirstLaunch = false;

    if (Asset == nullptr)
    { return; }

    APawn* ControlledActor = GetLocalPlayerPawn();
    if (ControlledActor == nullptr)
    { return; }

    const ACogEngineReplicator* Replicator = ACogEngineReplicator::GetLocalReplicator(*GetWorld());
    if (Replicator == nullptr)
    { return; }

    TArray<AActor*> Targets { ControlledActor };

    for (int32 i = Config->AppliedCheats.Num() - 1; i >= 0; i--)
    {
        if (const FCogEngineCheat* Cheat = FindCheatByName(Config->AppliedCheats[i], true))
        {
            Replicator->Server_ApplyCheat(ControlledActor, Targets, *Cheat);
        }
        else
        {
            //-----------------------------------------------------
            // This cheat doesn't exist anymore. We can remove it.
            //-----------------------------------------------------
            Config->AppliedCheats.RemoveAt(i);
        }
    }

    bHasReappliedCheats = true;
}

//--------------------------------------------------------------------------------------------------------------------------
bool FCogEngineWindow_Cheats::DrawTable()
{
    const bool Open = ImGui::BeginTable("Cheats", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoBordersInBodyUntilResize);
    ImGui::TableSetupColumn("Toggle", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Instant", ImGuiTableColumnFlags_WidthStretch);
    return Open;
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Cheats::RenderContent()
{
    Super::RenderContent();

    if (Config == nullptr)
    {
        ImGui::TextDisabled("Invalid Config");
        return;
    }

    AActor* SelectedActor = GetSelection();
    if (SelectedActor == nullptr)
    {
        ImGui::TextDisabled("Invalid Selection");
        return;
    }

    if (Asset == nullptr)
    {
        ImGui::TextDisabled("Invalid Asset");
        return;
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            ImGui::Checkbox("Group By Category", &Config->bGroupByCategories);

            ImGui::Checkbox("Use Two Columns", &Config->bUseTwoColumns);

            ImGui::Separator();

            ImGui::Checkbox("Reapply Cheats Between Plays", &Config->bReapplyCheatsBetweenPlays);
            if (Config->bReapplyCheatsBetweenPlays == false)
            {
                ImGui::BeginDisabled();
            }

            ImGui::Checkbox("Reapply Cheats Between Launches", &Config->bReapplyCheatsBetweenLaunches);
            if (Config->bReapplyCheatsBetweenPlays == false)
            {
                ImGui::EndDisabled();
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Reset Settings"))
            {
                ResetConfig();
            }

            ImGui::EndMenu();
        }

        ImGui::SameLine();

        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7);

        if (ImGui::BeginMenu("Filters"))
        {
            for (const FCogEngineCheatCategory& CheatCategory : Asset->CheatCategories)
            {
                const auto CategoryStr = StringCast<ANSICHAR>(*CheatCategory.Name);
                bool IsSelected = Config->SelectedCategories.Contains(CheatCategory.Name);
                if (ImGui::Checkbox(CategoryStr.Get(), &IsSelected))
                {
                    if (IsSelected)
                    {
                        Config->SelectedCategories.Add(CheatCategory.Name);
                    }
                    else
                    {
                        Config->SelectedCategories.Remove(CheatCategory.Name);
                    }
                }
            }
            ImGui::EndMenu();
        }

        FCogWindowWidgets::SearchBar("##Filter", Filter);

        ImGui::EndMenuBar();
    }

    APawn* ControlledActor = GetLocalPlayerPawn();

    bool OpenTable = false;
    if (Config->bGroupByCategories == false && Config->bUseTwoColumns)
    {
        OpenTable = DrawTable();
    }
    
    for (const FCogEngineCheatCategory& CheatCategory : Asset->CheatCategories)
    {
        const auto CategoryStr = StringCast<ANSICHAR>(*CheatCategory.Name);

        if (Config->SelectedCategories.Num() != 0 && Config->SelectedCategories.Contains(CheatCategory.Name) == false)
        { continue; }

        bool Open = true;
        if (Config->bGroupByCategories)
        {
            Open = FCogWindowWidgets::DarkCollapsingHeader(CategoryStr.Get(), ImGuiTreeNodeFlags_DefaultOpen);

            if (Open && Config->bUseTwoColumns)
            {
                DrawTable();
            }
        }


        if (Open)
        {
            if (Config->bUseTwoColumns)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
            }

            int Index = 0;
            for (const FCogEngineCheat& Cheat : CheatCategory.PersistentEffects)
            {
                AddCheat(Index, ControlledActor, SelectedActor, Cheat, true);
                Index++;
            }

            //----------------------------------------------------------------------------
            // Update the config of applied cheat to reapply them on the next launch. 
            // We do not update them only when the user input is pressed because
            // the state of the cheat is lagging when connected to a server. 
            // So we check if the array should be updated all the time.
            //----------------------------------------------------------------------------
            if (SelectedActor == ControlledActor)
            {
                for (const FCogEngineCheat& Cheat : CheatCategory.PersistentEffects)
                {
                    TArray<AActor*> Targets = { SelectedActor };
                    if (ACogEngineReplicator::IsCheatActiveOnTargets(Targets, Cheat) == ECogEngineCheat_ActiveState::Active)
                    {
                        Config->AppliedCheats.AddUnique(Cheat.Name);
                    }
                    else
                    {
                        Config->AppliedCheats.Remove(Cheat.Name);
                    }
                }
            }

            ImGui::TableNextColumn();

            Index = 0;
            for (const FCogEngineCheat& Cheat : CheatCategory.InstantEffects)
            {
                AddCheat(Index, ControlledActor, SelectedActor, Cheat, false);
                Index++;
            }

            if (Config->bGroupByCategories && Config->bUseTwoColumns)
            {
                ImGui::EndTable();
            }
        }
    }

    if (OpenTable)
    {
        ImGui::EndTable();
    }
}

//--------------------------------------------------------------------------------------------------------------------------
bool FCogEngineWindow_Cheats::AddCheat(const int32 Index, AActor* ControlledActor, AActor* SelectedActor, const FCogEngineCheat& Cheat, bool IsPersistent)
{
    const auto CheatName = StringCast<ANSICHAR>(*Cheat.Name);

    if (Filter.PassFilter(CheatName.Get()) == false)
    { return false; }

    ImGui::PushID(Index);

    FCogWindowWidgets::PushBackColor(FCogImguiHelper::ToImVec4(Cheat.CustomColor));

    const bool IsShiftDown      = (ImGui::GetCurrentContext()->IO.KeyMods & ImGuiMod_Shift) != 0;
    const bool IsAltDown        = (ImGui::GetCurrentContext()->IO.KeyMods & ImGuiMod_Alt) != 0;
    const bool IsControlDown    = (ImGui::GetCurrentContext()->IO.KeyMods & ImGuiMod_Ctrl) != 0;

    bool bIsPressed = false;
    if (IsPersistent)
    {
        TArray<AActor*> Targets = { SelectedActor };
        bool isEnabled = ACogEngineReplicator::IsCheatActiveOnTargets(Targets, Cheat) == ECogEngineCheat_ActiveState::Active;
        if (ImGui::Checkbox(CheatName.Get(), &isEnabled))
        {
            RequestCheat(ControlledActor, SelectedActor, Cheat, IsShiftDown, IsAltDown, IsControlDown);
            bIsPressed = true;
        }
    }
    else
    {
        if (ImGui::Button(CheatName.Get(), ImVec2(-1, 0)))
        {
            RequestCheat(ControlledActor, SelectedActor, Cheat, IsShiftDown, IsAltDown, IsControlDown);
            bIsPressed = true;
        }
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, IsShiftDown || IsAltDown || IsControlDown ? 0.5f : 1.0f),   "On Selection");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, IsShiftDown ? 1.0f : 0.5f),                                 "On Enemies    [SHIFT]");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, IsAltDown ? 1.0f : 0.5f),                                   "On Allies     [ALT]");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, IsControlDown ? 1.0f : 0.5f),                               "On Controlled [CTRL]");
        ImGui::EndTooltip();
    }

    FCogWindowWidgets::PopBackColor();

    ImGui::PopID();

    return bIsPressed;
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Cheats::RequestCheat(AActor* ControlledActor, AActor* SelectedActor, const FCogEngineCheat& Cheat, bool ApplyToEnemies, bool ApplyToAllies, bool ApplyToControlled)
{
    TArray<AActor*> Actors;

    if (ApplyToControlled)
    {
        Actors.Add(ControlledActor);
    }
    
    if (ApplyToEnemies || ApplyToAllies)
    {
        for (TActorIterator<ACharacter> It(GetWorld(), ACharacter::StaticClass()); It; ++It)
        {
            if (AActor* OtherActor = *It)
            {
                ECogCommonAllegiance Allegiance = ECogCommonAllegiance::Enemy;
                
                if (ICogCommonAllegianceActorInterface* AllegianceInterface = Cast<ICogCommonAllegianceActorInterface>(OtherActor))
                {
                    Allegiance = AllegianceInterface->GetAllegianceWithOtherActor(ControlledActor);
                }

                if ((ApplyToEnemies && (Allegiance == ECogCommonAllegiance::Enemy))
                    || (ApplyToAllies && (Allegiance == ECogCommonAllegiance::Friendly)))
                {
                    Actors.Add(OtherActor);
                }
            }
        }
    }

    if ((ApplyToControlled || ApplyToEnemies || ApplyToAllies) == false)
    {
        Actors.Add(SelectedActor);
    }

    if (ACogEngineReplicator* Replicator = ACogEngineReplicator::GetLocalReplicator(*GetWorld()))
    {
        Replicator->Server_ApplyCheat(ControlledActor, Actors, Cheat);
    }
    else
    {
        UE_LOG(LogCogImGui, Warning, TEXT("FCogAbilityWindow_Cheats::RequestCheat | Replicator not found"));
    }
}

//--------------------------------------------------------------------------------------------------------------------------
const FCogEngineCheat* FCogEngineWindow_Cheats::FindCheatByName(const FString& CheatName, const bool OnlyPersistentCheats)
{
    for (const FCogEngineCheatCategory& CheatCategory : Asset->CheatCategories)
    {
        for (const FCogEngineCheat& Cheat : CheatCategory.PersistentEffects)
        {
            if (Cheat.Name == CheatName)
            {
                return &Cheat;
            }
        }

        if (OnlyPersistentCheats)
        {
            continue;
        }

        for (const FCogEngineCheat& Cheat : CheatCategory.InstantEffects)
        {
            if (Cheat.Name == CheatName)
            {
                return &Cheat;
            }
        }


    }

    return nullptr;
}
