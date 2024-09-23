using Script.CoreUObject;
using Script.Engine;
using Script.SlateCore;
using Script.UMG;
using System;
using System.Collections.Generic;
using Script.GameBasic.UI.GM;

namespace Script.GameBasic
{
    public enum EGMCommandType
    {
        None,
        Button,
        CheckBox,
        NumberBox,
        StringBox,
        ComboBox,
        Group
    }

    public enum EGMCommandGroupType
    {
        None,
        HBox,
        VBox,
        WrapBox
    }

    public class IGMCommandBase
    {
        public string Name;
        public virtual EGMCommandType GetCommandType() { return EGMCommandType.None; }
    }

    public class FGMCommand_Button : IGMCommandBase
    {
        public override EGMCommandType GetCommandType() { return EGMCommandType.Button; }
        public Action OnClicked;
    }

    public class FGMCommand_CheckBox : IGMCommandBase
    {
        public override EGMCommandType GetCommandType() { return EGMCommandType.CheckBox; }
        public bool InitialState;
        public Action<bool> OnCheckStateChanged;
    }

    public class FGMCommand_NumberBox : IGMCommandBase
    {
        public override EGMCommandType GetCommandType() { return EGMCommandType.NumberBox; }
        public float InitialValue;
        public float MinimumValue;
        public float MaximumValue;
        public float Delta;
        public Action<float> OnNumberChanged;
    }

    public class FGMCommand_StringBox : IGMCommandBase
    {
        public override EGMCommandType GetCommandType() { return EGMCommandType.StringBox; }
        public string InitialString;
        public string PlaceholderString;
        public Action<string> OnStringCommit;
    }

    public class FGMCommand_ComboBox : IGMCommandBase
    {
        public override EGMCommandType GetCommandType() { return EGMCommandType.ComboBox; }
        public List<string> OptionList;
        public string InitialOption;
        public Action<string> OnSelectionChanged;
    }

    public class FGMCommand_Group : IGMCommandBase
    {
        public override EGMCommandType GetCommandType() { return EGMCommandType.Group; }
        public EGMCommandGroupType GroupType;
        public List<IGMCommandBase> SubCommandList;
    }

    public sealed class GMManager
    {
        private static readonly GMManager Instance = new GMManager();
        public static GMManager Get() { return Instance; }

        private FGMUIC GMUIC = null;

        private GMManager()
        {
            BeginCategory("Common");

            BeginGroup("Group A", EGMCommandGroupType.HBox);

            AddButtonCommand("Button", () =>
            {
                Console.WriteLine("Clicked!!!");
            });

            AddButtonCommand("Button", () =>
            {
                Console.WriteLine("Clicked!!!");
            });

            AddCheckBoxCommand("Check", false, (bool Value) =>
            {
                Console.WriteLine("Clicked!!!");
            });

            AddStringBoxCommand("String","","",(string Value) => 
            { 

            });

            AddNumberBoxCommand("Number", 0, 0, 100, 1, (float Value) =>
            {

            });

            AddComboBoxCommand("Combo", new List<string> { "A", "B" ,"C","D","E" }, "A", (string Value) =>
            {

            });

            EndGroup();

            BeginGroup("Group B", EGMCommandGroupType.HBox);

            AddButtonCommand("Button", () =>
            {
                Console.WriteLine("Clicked!!!");
            });

            AddButtonCommand("Button", () =>
            {
                Console.WriteLine("Clicked!!!");
            });

            AddCheckBoxCommand("Check", false, (bool Value) =>
            {
                Console.WriteLine("Clicked!!!");
            });

            AddStringBoxCommand("String", "", "", (string Value) =>
            {

            });

            AddNumberBoxCommand("Number", 0, 0, 100, 1, (float Value) =>
            {

            });

            AddComboBoxCommand("Combo", new List<string> { "A", "B", "C", "D", "E" }, "A", (string Value) =>
            {

            });

            EndGroup();

            EndCategory();

            BeginCategory("Other");

            AddButtonCommand("Button", () =>
            {
                Console.WriteLine("Clicked!!!");
            });

            EndCategory();

            EngineManager.Get().RegisterConsoleCommand("GM", "Show GM", () =>
            {
                Show();
            });
        }

        public void Show() { 
            if(GMUIC == null)
            {
                GMUIC = new FGMUIC();
            }
            GMUIC.Show();
        }

        public void Hide()
        {
            if (GMUIC != null)
            {
                GMUIC.Hide();
            }
        }

        internal List<string> CategoryOrder = new List<string>();
        internal Dictionary<string, List<IGMCommandBase>> GMCommands = new Dictionary<string, List<IGMCommandBase>>();
        internal string CurrentCategory = "Common";
        internal List<IGMCommandBase> CurrentCommandList;

        void BeginCategory(string CategoryName)
        {
            CurrentCategory = CategoryName;
            CategoryOrder.Add(CategoryName);
            GMCommands.Add(CurrentCategory, new List<IGMCommandBase>());
            CurrentCommandList = GMCommands[CurrentCategory];
        }

        void EndCategory()
        {
            CurrentCategory = "Common";
            CurrentCommandList = null;
        }

        void BeginGroup(string GroupName, EGMCommandGroupType GroupType = EGMCommandGroupType.WrapBox)
        {
            FGMCommand_Group Group = new FGMCommand_Group();
            Group.Name = GroupName;
            Group.GroupType = GroupType;
            Group.SubCommandList = new List<IGMCommandBase>();
            AddCommand(Group);
            CurrentCommandList = Group.SubCommandList;
        }

        public void EndGroup()
        {
            CurrentCommandList = GMCommands[CurrentCategory];
        }

        private void AddCommand(IGMCommandBase Command)
        {
            if (Command != null)
            {
                CurrentCommandList.Add(Command);
            }
            else
            {
                Console.WriteLine("Command is Invalid");
            }
        }

        public void AddButtonCommand(string Name, Action OnClicked)
        {
            FGMCommand_Button Command = new FGMCommand_Button();
            Command.Name = Name;
            Command.OnClicked = OnClicked;
            string CommandPath = CurrentCategory + "." + Name;
            AddCommand(Command);
            EngineManager.Get().RegisterConsoleCommand(CommandPath, Command.Name, Command.OnClicked);
        }

        public void AddCheckBoxCommand(string Name, bool InitialState, Action<bool> OnCheckStateChanged)
        {
            FGMCommand_CheckBox Command = new FGMCommand_CheckBox();
            Command.Name = Name;
            Command.InitialState = InitialState;
            Command.OnCheckStateChanged = OnCheckStateChanged;
            AddCommand(Command);
            string CommandPath = CurrentCategory + "." + Name;
            EngineManager.Get().RegisterConsoleCommand(CommandPath, Command.Name, (FString Str) =>
            {
                Command.OnCheckStateChanged(int.Parse(Str.ToString()) != 0);
            });
        }

        public void AddNumberBoxCommand(string Name, float InitialValue, float MinimumValue, float MaximumValue, float Delta, Action<float> OnNumberChanged)
        {
            FGMCommand_NumberBox Command = new FGMCommand_NumberBox();
            Command.Name = Name;
            Command.InitialValue = InitialValue;
            Command.MinimumValue = MinimumValue;
            Command.MaximumValue = MaximumValue;
            Command.Delta = Delta;
            Command.OnNumberChanged = OnNumberChanged;
            AddCommand(Command);
            string CommandPath = CurrentCategory + "." + Name;
            EngineManager.Get().RegisterConsoleCommand(CommandPath, Command.Name, (FString Str) =>
            {
                Command.OnNumberChanged(float.Parse(Str.ToString()));
            });
        }

        public void AddStringBoxCommand(string Name, string InitialString, string PlaceholderString, Action<string> OnStringCommit)
        {
            FGMCommand_StringBox Command = new FGMCommand_StringBox();
            Command.Name = Name;
            Command.InitialString = InitialString;
            Command.PlaceholderString = PlaceholderString;
            Command.OnStringCommit = OnStringCommit;
            AddCommand(Command);
            string CommandPath = CurrentCategory + "." + Name;
            EngineManager.Get().RegisterConsoleCommand(CommandPath, Command.Name, (FString Str) =>
            {
                Command.OnStringCommit(Str.ToString());
            });
        }

        public void AddComboBoxCommand(string Name, List<string> OptionList, string InitialOption, Action<string> OnSelectionChanged)
        {
            FGMCommand_ComboBox Command = new FGMCommand_ComboBox();
            Command.Name = Name;
            Command.OptionList = OptionList;
            Command.InitialOption = InitialOption;
            Command.OnSelectionChanged = OnSelectionChanged;
            AddCommand(Command);
            string CommandPath = CurrentCategory + "." + Name;
            EngineManager.Get().RegisterConsoleCommand(CommandPath, Command.Name, (FString Str) =>
            {
                Command.OnSelectionChanged(Str.ToString());
            });
        }

    }
}