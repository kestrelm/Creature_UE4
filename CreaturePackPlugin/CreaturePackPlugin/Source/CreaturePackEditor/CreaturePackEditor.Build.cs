using System.IO;

namespace UnrealBuildTool.Rules
{
    public class CreaturePackEditor : ModuleRules
    {
        private string ModulePath
        {
            get { 

                
                string ModuleCSFilename = RulesCompiler.GetFileNameFromType(GetType());
                string ModuleBaseDirectory = Path.GetDirectoryName(ModuleCSFilename);

                return ModuleBaseDirectory;
            }
        }


        public CreaturePackEditor(ReadOnlyTargetRules Target)
            : base(Target)
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
            PublicIncludePaths.AddRange(new string[] { ModulePath + "/Public"});
            PrivateIncludePaths.AddRange(new string[] { ModulePath + "/Public", ModulePath + "/Private"});

            PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "CreaturePackRuntimePlugin", "UnrealEd", "PropertyEditor","AssetTools","EditorStyle","KismetWidgets","GraphEditor","AnimGraph" });

            PrivateDependencyModuleNames.AddRange(new string[] { "RHI", "RenderCore", "ShaderCore", "CreaturePackRuntimePlugin", "SlateCore", "Slate", "AssetTools", "GraphEditor", "AnimGraph" });

           // LoadCreatureLib(Target);
        }
    }
}
