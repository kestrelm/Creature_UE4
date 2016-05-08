using System.IO;

namespace UnrealBuildTool.Rules
{
    public class CreaturePackEditor : ModuleRules
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


        public CreaturePackEditor(TargetInfo Target)
        {
            PublicIncludePaths.AddRange(new string[] { "CreaturePackEditor/Public", "AssetTools/Public","GraphEditorActions/Public","AnimGraph/Public"});
            PrivateIncludePaths.AddRange(new string[] { "CreaturePackEditor/Public", "CreaturePackEditor/Private","UnrealEd/Private/Settings","Editor/AnimGraph/Private" });

            PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "CreaturePackRuntimePlugin", "UnrealEd", "PropertyEditor", "Paper2DEditor","AssetTools","EditorStyle","KismetWidgets","GraphEditor","AnimGraph" });

            PrivateDependencyModuleNames.AddRange(new string[] { "RHI", "RenderCore", "ShaderCore", "CreaturePackRuntimePlugin", "SlateCore", "Slate", "AssetTools", "GraphEditor", "AnimGraph" });

           // LoadCreatureLib(Target);
        }
    }
}
