#pragma once

#include "CoreMinimal.h"
#include "imgui.h"
#include "UObject/ReflectedTypeAccessors.h"

#include <Templates/SubclassOf.h>

class AActor;
class APawn;
class FEnumProperty;
class UCollisionProfile;
class UEnum;
class UObject;
enum class ECheckBoxState : uint8;
enum ECollisionChannel : int;
struct FCogImGuiKeyInfo;
struct FKeyBind;

using FCogWindowActorContextMenuFunction = TFunction<void(AActor& Actor)>;

class COGWINDOW_API FCogWindowWidgets
{
public:

    static bool BeginTableTooltip();

    static void EndTableTooltip();

    static bool ItemTooltipWrappedText(const char* InText);
    
    static bool BeginItemTableTooltip();

    static void EndItemTableTooltip();

    static void ThinSeparatorText(const char* Label);

    static bool DarkCollapsingHeader(const char* InLabel, ImGuiTreeNodeFlags InFlags);

    static void ProgressBarCentered(float Fraction, const ImVec2& Size, const char* Overlay);

    static bool ToggleMenuButton(bool* Value, const char* Text, const ImVec4& TrueColor);

    static bool ToggleButton(bool* Value, const char* Text, const ImVec4& TrueColor, const ImVec4& FalseColor, const ImVec2& Size = ImVec2(0, 0));

    static bool ToggleButton(bool* Value, const char* TextTrue, const char* TextFalse, const ImVec4& TrueColor, const ImVec4& FalseColor, const ImVec2& Size = ImVec2(0, 0));

    static bool MultiChoiceButton(const char* Label, bool IsSelected, const ImVec2& Size = ImVec2(0, 0));

    static bool MultiChoiceButtonsInt(TArray<int32>& Values, int32& Value, const ImVec2& Size = ImVec2(0, 0));

    static bool MultiChoiceButtonsFloat(TArray<float>& Values, float& Value, const ImVec2& Size = ImVec2(0, 0));

    static void SliderWithReset(const char* Name, float* Value, float Min, float Max, const float& ResetValue, const char* Format);

    static void HelpMarker(const char* Text);

    static void PushStyleCompact();
    
    static void PopStyleCompact();

    static void AddTextWithShadow(ImDrawList* DrawList, const ImVec2& Position, ImU32 Color, const char* TextBegin, const char* TextEnd = NULL);

    static bool SearchBar(const char* InLabel, ImGuiTextFilter& InFilter, float InWidth = -1.0f);

    static void PushBackColor(const ImVec4& Color);

    static void PopBackColor();

    static float GetShortWidth();

    static void SetNextItemToShortWidth();

    static float GetFontWidth();

    template<typename EnumType>
    static bool ComboboxEnum(const char* Label, const EnumType CurrentValue, EnumType& NewValue);

    template<typename EnumType>
    static bool ComboboxEnum(const char* Label, EnumType& Value);

    static bool ComboboxEnum(const char* Label, UEnum* Enum, int64 CurrentValue, int64& NewValue);
    
    static bool ComboboxEnum(const char* Label, UObject* Object, const char* FieldName, uint8* PointerToEnumValue);
    
    static bool ComboboxEnum(const char* Label, const FEnumProperty* EnumProperty, uint8* PointerToEnumValue);

    static bool CheckBoxState(const char* Label, ECheckBoxState& State, bool ShowTooltip = true);

    static bool InputKey(const char* Label, FCogImGuiKeyInfo& KeyInfo);

    static bool InputKey(FCogImGuiKeyInfo& KeyInfo);
    
    static bool KeyBind(FKeyBind& KeyBind);

    static bool ButtonWithTooltip(const char* Text, const char* Tooltip);

    static bool DeleteArrayItemButton();

    static bool ComboCollisionChannel(const char* Label, ECollisionChannel& Channel);

    static bool CollisionProfileChannel(const UCollisionProfile& CollisionProfile, int32 ChannelIndex, FColor& ChannelColor, int32& Channels);

    static bool CollisionProfileChannels(int32& Channels);

    static bool MenuActorsCombo(const char* StrID, AActor*& NewSelection, const UWorld& World, TSubclassOf<AActor> ActorClass, const FCogWindowActorContextMenuFunction& ContextMenuFunction = nullptr);

    static bool MenuActorsCombo(const char* StrID, AActor*& NewSelection, const UWorld& World, const TArray<TSubclassOf<AActor>>& ActorClasses, int32& SelectedActorClassIndex, ImGuiTextFilter* Filter, const APawn* LocalPlayerPawn, const FCogWindowActorContextMenuFunction& ContextMenuFunction = nullptr);

    static bool ActorsListWithFilters(AActor*& NewSelection, const UWorld& World, const TArray<TSubclassOf<AActor>>& ActorClasses, int32& SelectedActorClassIndex, ImGuiTextFilter* Filter, const APawn* LocalPlayerPawn, const FCogWindowActorContextMenuFunction& ContextMenuFunction = nullptr);

    static bool ActorsList(AActor*& NewSelection, const UWorld& World, const TSubclassOf<AActor> ActorClass, const ImGuiTextFilter* Filter = nullptr, const APawn* LocalPlayerPawn = nullptr, const FCogWindowActorContextMenuFunction& ContextMenuFunction = nullptr);

    static void ActorContextMenu(AActor& Selection, const FCogWindowActorContextMenuFunction& ContextMenuFunction);

    static void ActorFrame(const AActor& Actor);

    static void SmallButton(const char* Text, const ImVec4& Color);

    static bool InputText(const char* Text, FString& Value,  ImGuiInputTextFlags InFlags = 0, ImGuiInputTextCallback InCallback = nullptr, void* InUserData = nullptr);

    static bool InputTextWithHint(const char* InText, const char* InHint, FString& InValue, ImGuiInputTextFlags InFlags = 0, ImGuiInputTextCallback InCallback = nullptr, void* InUserData = nullptr);

    static bool BeginRightAlign(const char* Id);

    static void EndRightAlign();

    static void MenuItemShortcut(const char* Id, const FString& Text);

    static bool BrowseToAssetButton(const UObject* InAsset, const ImVec2& InSize = ImVec2(0, 0));

    static bool BrowseToObjectAssetButton(const UObject* InObject, const ImVec2& InSize = ImVec2(0, 0));

    static bool OpenAssetButton(const UObject* InAsset, const ImVec2& InSize = ImVec2(0, 0));

    static bool OpenObjectAssetButton(const UObject* InObject, const ImVec2& InSize = ImVec2(0, 0));

    static void RenderClosebutton(const ImVec2& InPos);

};

template<typename EnumType>
bool FCogWindowWidgets::ComboboxEnum(const char* Label, const EnumType CurrentValue, EnumType& NewValue)
{
    int64 NewValueInt;
    if (ComboboxEnum(Label, StaticEnum<EnumType>(), (int64)CurrentValue, NewValueInt))
    {
        NewValue = (EnumType)NewValueInt;
        return true;
    }

    return false;
}

template<typename EnumType>
bool FCogWindowWidgets::ComboboxEnum(const char* Label, EnumType& Value)
{
    return ComboboxEnum(Label, Value, Value);
}