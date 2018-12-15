using System.IO;

namespace UnrealBuildTool.Rules
{
    public class CreatureEditor : ModuleRules
    {
        private string ModulePath
        {
            //get { return Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name)); }
            get { 

                
                string ModuleCSFilename = RulesCompiler.GetFileNameFromType(GetType());
                string ModuleBaseDirectory = Path.GetDirectoryName(ModuleCSFilename);

                return ModuleBaseDirectory;
            }
        }

        private string ThirdPartyPath
        {
            get { return "F:\\BRS2D\\Project\\brs2d\\Plugins\\ThirdParty"; }
        }



        public CreatureEditor(ReadOnlyTargetRules Target)
            : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            PublicIncludePaths.AddRange(new string[] { ModulePath + "/Public", "Editor/GraphEditor/Public","Editor/AnimGraph/Public"});
            PrivateIncludePaths.AddRange(new string[] { ModulePath + "/Private" });

            PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "CreaturePlugin", "UnrealEd", "PropertyEditor", "AssetTools","EditorStyle","KismetWidgets","GraphEditor" });

            PrivateDependencyModuleNames.AddRange(new string[] { "RHI", "RenderCore", "ShaderCore", "CreaturePlugin", "SlateCore", "Slate", "AssetTools", "GraphEditor", "Json", "JsonUtilities", "AdvancedPreviewScene" });

           // LoadCreatureLib(Target);
        }
    }
}
