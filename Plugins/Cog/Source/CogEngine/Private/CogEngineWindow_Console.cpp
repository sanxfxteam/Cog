#include "CogEngineWindow_Console.h"

#include "CogImguiHelper.h"
#include "CogWindowManager.h"
#include "CogWindowWidgets.h"
#include "imgui.h"
#include "imgui_internal.h"

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Console::RenderHelp()
{
    ImGui::Text("This window can be used as a replacement of the Unreal console.");
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Console::Initialize()
{
    FCogWindow::Initialize();

    Config = GetConfig<UCogEngineConfig_Console>();

    bNoPadding = true;
    bHasMenu = true;
    bHasWidget = true;
    bIsWidgetVisible = true;
    SelectedCommandIndex = -1;
    
    RefreshCommandList();
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Console::RenderContent()
{
    Super::RenderContent();

    const APlayerController* PlayerController = GetLocalPlayerController();
    if (PlayerController == nullptr)
    {
        return;
    }

    if (ImGui::BeginMenuBar())
    {
        RenderMenu();
        ImGui::EndMenuBar();
    }

    ImGui::Spacing();
    
    if (Config->DockInputInMenuBar == false)
    {
        ImGui::SetNextItemWidth(-1);
        RenderInput();
    }

    RenderCommandList();
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Console::RenderMainMenuWidget()
{
    bIsRenderingWidget = true;
    
    const ImGuiContext& g = *GImGui;
    const ImGuiStyle& Style = g.Style;
    const ImGuiWindow* Window = ImGui::GetCurrentWindow();
    ImVec2 TooltipPos = Window->DC.CursorPos;
    TooltipPos.y += Window->MenuBarHeight;

    ImGui::SetNextItemWidth(Config->WidgetWidth);

    RenderInput();
    const bool IsTextInputActive = ImGui::IsItemActive();
    
    if (Config->FocusWidgetWhenAppearing && ImGui::IsWindowAppearing())
    {
        SelectedCommandIndex = -1;
        RefreshCommandList();
        ImGui::ActivateItemByID(InputIdOnWidgetMode);
    }

    if (ImGui::BeginPopupContextItem())
    {
        RenderMenu();
        ImGui::EndPopup();
    }
    
    if (bPopupCommandListOnWidgetMode)
    {
        ImGui::SetNextWindowBgAlpha(g.Style.Colors[ImGuiCol_PopupBg].w * 0.90f);
        ImGui::SetNextWindowSize(ImVec2(Config->WidgetWidth, ImGui::GetFontSize() * 30), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(TooltipPos, ImGuiCond_Always);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoFocusOnAppearing; // We want the console input text to keep the focus.

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        if (ImGui::Begin("ConsoleTooltip", nullptr, flags))
        {
            RenderCommandList();
            
            if (ImGui::BeginPopupContextWindow())
            {
                RenderMenu();
                ImGui::EndPopup();
            }
            
            const bool IsWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

            if (IsTextInputActive == false && IsWindowFocused == false && bRequestInputFocus == false)
            {
                bPopupCommandListOnWidgetMode = false;
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    bIsRenderingWidget = false;
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Console::RenderMenu()
{
    if (ImGui::BeginMenu("Options"))
    {
        FCogWindowWidgets::ThinSeparatorText("General");
        
        if (ImGui::Checkbox("Sort Commands", &Config->SortCommands))
        {
            RefreshCommandList();
        }

        // if (ImGui::Checkbox("Use Clipper", &Config->UseClipper))
        // {
        //     RefreshCommandList();
        // }

        FCogWindowWidgets::SetNextItemToShortWidth();
        if (ImGui::SliderInt("Completion Minimum Characters", &Config->CompletionMinimumCharacters, 0, 3))
        {
            RefreshCommandList();
        }
        
        FCogWindowWidgets::SetNextItemToShortWidth();
        if (ImGui::SliderInt("Num History Commands", &Config->NumHistoryCommands, 0, 100))
        {
            RefreshCommandList();
        }
        
        ImGui::ColorEdit4("History Color", (float*)&Config->HistoryColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreviewHalf);

        FCogWindowWidgets::ThinSeparatorText("Window");

        if (ImGui::Checkbox("Dock Input in Menu Bar", &Config->DockInputInMenuBar))
        {
            RefreshCommandList();
        }

        FCogWindowWidgets::ThinSeparatorText("Widget");

        if (ImGui::Checkbox("Focus Console Widget When Appearing", &Config->FocusWidgetWhenAppearing))
        {
            RefreshCommandList();
        }
        
        FCogWindowWidgets::SetNextItemToShortWidth();
        ImGui::SliderInt("Widget Width", &Config->WidgetWidth, 0, 1000);

        ImGui::EndMenu();
    }

    if (bIsRenderingWidget == false && Config->DockInputInMenuBar)
    {
        ImGui::SetNextItemWidth(-1);
        RenderInput();
    }

    //ImGui::Text("%d", SelectedCommandIndex);
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Console::RenderInput()
{
    constexpr ImGuiInputTextFlags InputFlags =
        ImGuiInputTextFlags_EnterReturnsTrue
        | ImGuiInputTextFlags_EscapeClearsAll
        | ImGuiInputTextFlags_CallbackCompletion
        | ImGuiInputTextFlags_CallbackHistory
        | ImGuiInputTextFlags_CallbackEdit
        | ImGuiInputTextFlags_CallbackAlways;
    
    if (FCogWindowWidgets::InputTextWithHint("##Command", "Command", CurrentUserInput, InputFlags, &OnTextInputCallbackStub, this))
    {
        ExecuteCommand(CurrentUserInput);
        bRequestInputFocus = true;
    }

    if (bIsRenderingWidget)
    {
        InputIdOnWidgetMode = ImGui::GetItemID();
    }
    
    ImGui::SetItemDefaultFocus();

    //-------------------------------------------------------------------------------------------------
    // In Widget mode, do not want to show the command list as soon as the input text has focus,
    // but wait  for the user to click on the input text (or interact with it). This is because
    // we want the text input to always have focus so the user can directly type text if he wants to,
    // but if he doesn't the command list should not clutter the screen.
    //-------------------------------------------------------------------------------------------------
    if (bIsRenderingWidget && ImGui::IsItemActive() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        bPopupCommandListOnWidgetMode = true;
    }
    
    if (ImGui::IsItemActive() == false && bRequestInputFocus)
    {
        ImGui::SetKeyboardFocusHere(-1);
    }

    if (ImGui::IsItemActive())
    {
         bRequestInputFocus = false;
    }
}

//--------------------------------------------------------------------------------------------------------------------------
int FCogEngineWindow_Console::OnTextInputCallback(ImGuiInputTextCallbackData* InData)
{
    bool DoCompletion = false;
    if (InData->EventFlag == ImGuiInputTextFlags_CallbackHistory)
    {
        DoCompletion = true;

        if (InData->EventKey == ImGuiKey_UpArrow)
        {
            SelectedCommandIndex -= 1;
            bScroll = true;

            if (SelectedCommandIndex < 0)
            {
                SelectedCommandIndex = CommandList.Num() - 1;
            }
        }
        else if (InData->EventKey == ImGuiKey_DownArrow)
        {
            SelectedCommandIndex += 1;
            bScroll = true;

            if (SelectedCommandIndex >= CommandList.Num())
            {
                SelectedCommandIndex = 0;
            }
        }
    }
    else if (InData->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
    {
        DoCompletion = true;
    }
    else  if (InData->EventFlag == ImGuiInputTextFlags_CallbackEdit)
    {
        CurrentUserInput = FString(InData->Buf);
        RefreshCommandList();      

        if (bIsRenderingWidget)
        {
            bPopupCommandListOnWidgetMode = true;
        }
    }
    else if (InData->EventFlag == ImGuiInputTextFlags_CallbackAlways)
    {
        if (bSetBufferToSelectedCommand)
        {
            DoCompletion = true;
            bSetBufferToSelectedCommand = false;
        }
    }

    if (DoCompletion && CommandList.IsValidIndex(SelectedCommandIndex))
    {
        const FString SelectedCommand = CommandList[SelectedCommandIndex];
        const FString CleanupSelectedCommand = SelectedCommand.TrimEnd(); 
        const auto& CommandStr = StringCast<ANSICHAR>(*CleanupSelectedCommand);
        InData->DeleteChars(0, InData->BufTextLen);
        InData->InsertChars(0, CommandStr.Get());
        InData->InsertChars(InData->CursorPos, " ");

        if (bIsRenderingWidget)
        {
            bPopupCommandListOnWidgetMode = true;
        }
    }
    
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------------
int FCogEngineWindow_Console::OnTextInputCallbackStub(ImGuiInputTextCallbackData* InData)
{
    FCogEngineWindow_Console& ConsoleWindow	= *static_cast<FCogEngineWindow_Console*>(InData->UserData);
    return ConsoleWindow.OnTextInputCallback(InData);
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Console::RenderCommandHelp()
{

}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Console::RenderCommandList()
{
    const float HelpHeight = ImGui::GetFontSize() * 5;
    const float Indent = ImGui::GetFontSize() * 0.5f;

    const ImVec2 Size = IsWindowRenderedInMainMenu() ? ImVec2(0, ImGui::GetFontSize() * 20) : ImVec2(0.0f, -HelpHeight);
    if (ImGui::BeginChild("Commands", Size, ImGuiChildFlags_NavFlattened | ImGuiChildFlags_ResizeY))
    {
        ImGui::Indent(Indent);

        int32 Index = 0;

        ImGuiListClipper Clipper;
        Clipper.Begin(CommandList.Num());
        while (Clipper.Step())
        {
            const int32 Start = Config->UseClipper ? Clipper.DisplayStart : 0;
            const int32 End = Config->UseClipper ? Clipper.DisplayEnd : CommandList.Num();
        
            for (Index = Start; Index < End; Index++)
            {
                if (CommandList.IsValidIndex(Index))
                {
                    ImGui::PushID(Index);
                    const FString& CommandName = CommandList[Index];
                    RenderCommand(CommandName, Index);
                    ImGui::PopID();
                }
            }

            if (Config->UseClipper == false)
            { break; }
        }
        Clipper.End();

        // If any is available, draw an additional command below the clipper to be able to scroll when pressing bottom
        if (CommandList.IsValidIndex(Index + 1))
        {
            const FString& Command = CommandList[Index + 1];
            RenderCommand(Command, Index);
        }

        ImGui::Unindent(Indent);
    }
    ImGui::EndChild();

    //--------------------------------------------------------------------
    // Render Help
    //--------------------------------------------------------------------
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
    if (ImGui::BeginChild("Help", ImVec2(0.0f, ImGui::GetContentRegionAvail().y)))
    {
        ImGui::Spacing();
        ImGui::BeginDisabled();
        ImGui::Indent(Indent);

        if (CommandList.IsValidIndex(SelectedCommandIndex))
        {
            const FString SelectedCommand = CommandList[SelectedCommandIndex];
            const FString Help = GetConsoleCommandHelp(SelectedCommand);
            const auto& HelpStr = StringCast<ANSICHAR>(*Help);
            ImGui::TextWrapped(HelpStr.Get());
        }

        ImGui::Unindent(Indent);
        ImGui::EndDisabled();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
}


//--------------------------------------------------------------------------------------------------------------------------
FString FCogEngineWindow_Console::GetConsoleCommandHelp(const FString& InCommandName)
{
    if (InCommandName.IsEmpty() == false)
    {
        TArray<FString> CommandSplitWithSpaces;
        InCommandName.ParseIntoArrayWS(CommandSplitWithSpaces);

        if (CommandSplitWithSpaces.Num() > 0)
        {
            if (const IConsoleObject* ConsoleObject = IConsoleManager::Get().FindConsoleObject(*CommandSplitWithSpaces[0]))
            {
                return FString(ConsoleObject->GetHelp());
            }
        }
    }
    
    return FString("Unknown command.");
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Console::RenderCommand(const FString& CommandName, const int32 Index)
{
    const auto& CommandNameStr = StringCast<ANSICHAR>(*CommandName);

    bool IsSelected = Index == SelectedCommandIndex;

    // ImGui::Text("%d - ", Index);
    // ImGui::SameLine();

    if (Index < NumHistoryCommands)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, FCogImguiHelper::ToImVec4(Config->HistoryColor));
    }

    ImGuiSelectableFlags Flags =
        ImGuiSelectableFlags_AllowDoubleClick   // Double click executes the selected command
        | ImGuiSelectableFlags_SelectOnClick;   // Need to focus the console text input right away, otherwise the Selectable take back the focus on mouse release 
    
    if (ImGui::Selectable(CommandNameStr.Get(), &IsSelected,  Flags))
    {
        SelectedCommandIndex = Index;
        bRequestInputFocus = true;
        bSetBufferToSelectedCommand = true;
        
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            ExecuteCommand(CommandName);
        }
    }

    if (ImGui::BeginItemTooltip())
    {
        const FString Help = GetConsoleCommandHelp(CommandName);
        const auto& HelpStr = StringCast<ANSICHAR>(*Help);

        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(HelpStr.Get());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    
    if (Index < NumHistoryCommands)
    {
        ImGui::PopStyleColor();
    }

    if (NumHistoryCommands > 0 && Index == NumHistoryCommands - 1)
    {
        ImGui::Separator();
    }
    
    if (IsSelected)
    {
        if (bScroll)
        {
            ImGui::SetScrollHereY(1.0f);
            bScroll = false;
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Console::RefreshCommandList()
{
    FString CurrentUserInputWithoutArgs = CurrentUserInput;

    //------------------------------------------------------------------------------------------------------
    // Split the user input with spaces tp get the first part of the command so the completion is made on
    // "Cog.Cheat" instead of "Cog.Cheat God", as the later would return no results 
    //------------------------------------------------------------------------------------------------------
    TArray<FString> UserInputSplitWithSpaces;
    CurrentUserInput.ParseIntoArrayWS(UserInputSplitWithSpaces);
    if (UserInputSplitWithSpaces.Num() > 0)
    {
        CurrentUserInputWithoutArgs = UserInputSplitWithSpaces[0];
    }
        
    CommandList.Empty();

    TArray<FString> AllHistory;
    IConsoleManager::Get().GetConsoleHistory(TEXT(""), AllHistory);

    NumHistoryCommands = 0;
    for (int32 i = AllHistory.Num() - 1; i >= 0; i--)
    {
        FString Command = AllHistory[i];
        if (Command.IsEmpty())
        { continue; }

        if (CurrentUserInput.IsEmpty() == false && Command.StartsWith(CurrentUserInput) == false)
        { continue; }

        if (CommandList.Num() >= Config->NumHistoryCommands)
        { break; }

        CommandList.Add(Command);
        NumHistoryCommands++;
    }

    TArray<FString> Commands; 
    if (CurrentUserInputWithoutArgs.Len() >= Config->CompletionMinimumCharacters)
    {
        auto OnConsoleObject = [&](const TCHAR *InName, const IConsoleObject* InConsoleObject)
        {
            if (InConsoleObject->TestFlags(ECVF_Unregistered) || InConsoleObject->TestFlags(ECVF_ReadOnly))
            { return; }

            Commands.Add(InName);
        };

        IConsoleManager::Get().ForEachConsoleObjectThatStartsWith(FConsoleObjectVisitor::CreateLambda(OnConsoleObject), *CurrentUserInputWithoutArgs);
    }

    if (Config->SortCommands)
    {
        Commands.Sort();
    }

    CommandList.Append(Commands);

    //-------------------------------------------------------------------------------------
    // Reset to -1 so the next down arrow will select the first entry in history/command
    //-------------------------------------------------------------------------------------
    SelectedCommandIndex = -1;
}

//--------------------------------------------------------------------------------------------------------------------------
void FCogEngineWindow_Console::ExecuteCommand(const FString& InCommand)
{
    FString CleanupCommand = InCommand.TrimEnd(); 
    if (CleanupCommand.IsEmpty() == false)
    {
        IConsoleManager::Get().AddConsoleHistoryEntry(TEXT(""), *CleanupCommand);
        GEngine->DeferredCommands.Add(CleanupCommand);
    }    

    if (bIsRenderingWidget)
    {
        bPopupCommandListOnWidgetMode = false;
    }
    
    CurrentUserInput = FString();
    RefreshCommandList();
}


