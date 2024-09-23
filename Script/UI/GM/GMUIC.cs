using Script.CoreUObject;
using Script.Engine;
using Script.SlateCore;
using Script.UMG;
using System;
using System.Collections.Generic;
using Script.GameBasic.UI.GM;

namespace Script.GameBasic
{
    class FGMUIC : UIController
    {
        public UMG_GM_C GMUI;
        public FGMUIC()
        {
            GMUI = CreateView<UMG_GM_C>();
        }
        protected override void OnInitialize()
        {
            GMUI.Button.OnClicked.Add(GMUI, () => {
                this.Hide(); 
            });
            RecreateGmPanel();
        }

        protected override void OnShowEvent()
        {
            UIManager.Get().PlayerController.bShowMouseCursor = true;
            base.OnShowEvent();
        }

        protected override void OnHideEvent()
        {
            UIManager.Get().PlayerController.bShowMouseCursor = false;
            base.OnHideEvent();
        }

        protected void RecreateGmPanel()
        {
            foreach (string Category in GMManager.Get().CategoryOrder)
            {
                UMG_GMCategoryItem_C CategoryItem = CreateView<UMG_GMCategoryItem_C>(false);
                GMUI.CategoryVBox.AddChildToVerticalBox(CategoryItem);
                CategoryItem.TextBlock.SetText(Category);
                CategoryItem.Button.OnClicked.Add(GMUI, () => { 
                    SetCurrentCategory(Category);
                });
            }
            SetCurrentCategory("Common");
        }

        void SetCurrentCategory(string Category)
        {
            GMUI.GmVBox.ClearChildren();
            foreach(var Command in GMManager.Get().GMCommands[Category])
            {
                GenerateItemFroCommand(Command, (UWidget Widget) =>
                {
                    GMUI.GmVBox.AddChildToVerticalBox(Widget);
                });
            }
        }

        void GenerateItemFroCommand(IGMCommandBase Command, Action<UWidget> WidgetSetupAction)
        {
            switch (Command.GetCommandType())
            {
                case EGMCommandType.None:
                    break;
                case EGMCommandType.Button:
                    {
                        FGMCommand_Button Cmd = Command as FGMCommand_Button;
                        UMG_GMButtonItem_C Item = CreateView<UMG_GMButtonItem_C>(false);
                        WidgetSetupAction(Item);
                        Item.TextBlock.SetText(Cmd.Name);
                        Item.Button.OnClicked.Add(Item, Cmd.OnClicked);
                        break;
                    }
                case EGMCommandType.CheckBox:
                    {
                        FGMCommand_CheckBox Cmd = Command as FGMCommand_CheckBox;
                        UMG_GMCheckItem_C Item = CreateView<UMG_GMCheckItem_C>(false);
                        WidgetSetupAction(Item);
                        Item.LbName.SetText(Cmd.Name);
                        Item.CheckBox.SetCheckedState(Cmd.InitialState ? ECheckBoxState.Checked : ECheckBoxState.Unchecked);
                        Item.CheckBox.OnCheckStateChanged.Add(Item, Cmd.OnCheckStateChanged);
                        break;
                    }
                case EGMCommandType.NumberBox:
                    {
                        FGMCommand_NumberBox Cmd = Command as FGMCommand_NumberBox;
                        UMG_GMNumberItem_C Item = CreateView<UMG_GMNumberItem_C>(false);
                        WidgetSetupAction(Item);
                        Item.LbName.SetText(Cmd.Name);
                        Item.SpinBox.SetValue(Cmd.InitialValue);
                        Item.SpinBox.SetMaxSliderValue(Cmd.MaximumValue);
                        Item.SpinBox.SetMinValue(Cmd.MinimumValue);
                        Item.SpinBox.SetMinSliderValue(Cmd.MinimumValue);
                        Item.SpinBox.SetDelta(Cmd.Delta);
                        Item.SpinBox.OnValueChanged.Add(Item, Cmd.OnNumberChanged);
                        break;
                    }
                case EGMCommandType.StringBox:
                    {
                        FGMCommand_StringBox Cmd = Command as FGMCommand_StringBox;
                        UMG_GMStringItem_C Item = CreateView<UMG_GMStringItem_C>(false);
                        WidgetSetupAction(Item);
                        Item.LbName.SetText(Cmd.Name);
                        Item.TextBox.SetText(Cmd.InitialString);
                        Item.TextBox.SetHintText(Cmd.PlaceholderString);
                        break;
                    }
                case EGMCommandType.ComboBox:
                    {
                        FGMCommand_ComboBox Cmd = Command as FGMCommand_ComboBox;
                        UMG_GMComboItem_C Item = CreateView<UMG_GMComboItem_C>(false);
                        WidgetSetupAction(Item);
                        Item.LbName.SetText(Cmd.Name);
                        foreach (var Opt in Cmd.OptionList)
                        {
                            Item.ComboBox.AddOption(Opt);
                        }
                        Item.ComboBox.SetSelectedOption(Cmd.InitialOption);
                        Item.ComboBox.OnSelectionChanged.Add(Item, (FString Opt, ESelectInfo SelectInfo) =>
                        {
                            Cmd.OnSelectionChanged(Opt.ToString());
                        });
                        break;
                    }
                case EGMCommandType.Group:
                    {
                        FGMCommand_Group Cmd = Command as FGMCommand_Group;
                        UMG_GroupHeader_C Header = CreateView<UMG_GroupHeader_C>(false);
                        WidgetSetupAction(Header);
                        Header.LbName.SetText(Cmd.Name);
                        if(Cmd.GroupType == EGMCommandGroupType.WrapBox)
                        {
                            UWrapBox Item = Unreal.NewObject<UWrapBox>();
                            WidgetSetupAction(Item);
                            foreach (var SubCmd in Cmd.SubCommandList)
                            {
                                GenerateItemFroCommand(SubCmd, (UWidget Widget) =>
                                {
                                    Item.AddChildToWrapBox(Widget);
                                });
                            }
                        }
                        else if (Cmd.GroupType == EGMCommandGroupType.VBox)
                        {
                            UVerticalBox Item = Unreal.NewObject<UVerticalBox>();
                            WidgetSetupAction(Item);
                            foreach (var SubCmd in Cmd.SubCommandList)
                            {
                                GenerateItemFroCommand(SubCmd, (UWidget Widget) =>
                                {
                                    Item.AddChildToVerticalBox(Widget);
                                });
                            }
                        }
                        else if (Cmd.GroupType == EGMCommandGroupType.HBox)
                        {
                            UHorizontalBox Item = Unreal.NewObject<UHorizontalBox>();
                            WidgetSetupAction(Item);
                            foreach (var SubCmd in Cmd.SubCommandList)
                            {
                                GenerateItemFroCommand(SubCmd, (UWidget Widget) =>
                                {
                                    Item.AddChildToHorizontalBox(Widget);
                                });
                            }
                        }
                        break;
                    }
            }
        }
    }
}