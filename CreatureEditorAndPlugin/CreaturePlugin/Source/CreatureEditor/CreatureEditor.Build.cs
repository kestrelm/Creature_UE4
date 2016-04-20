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



        public CreatureEditor(TargetInfo Target)
        {
            PublicIncludePaths.AddRange(new string[] { "CreatureEditor/Public", "AssetTools/Public","GraphEditorActions/Public","AnimGraph/Public"});
            PrivateIncludePaths.AddRange(new string[] { "CreatureEditor/Private","UnrealEd/Private/Settings","Editor/AnimGraph/Private" });

            PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "CreaturePlugin", "UnrealEd", "PropertyEditor", "Paper2DEditor","AssetTools","EditorStyle","KismetWidgets","GraphEditor","AnimGraph" });

            PrivateDependencyModuleNames.AddRange(new string[] { "RHI", "RenderCore", "ShaderCore", "CreaturePlugin", "SlateCore", "Slate", "AssetTools", "GraphEditor", "AnimGraph" });

           // LoadCreatureLib(Target);
        }
    }
}
