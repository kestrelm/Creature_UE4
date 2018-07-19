using System.IO;

namespace UnrealBuildTool.Rules
{
    public class CreaturePackRuntimePlugin : ModuleRules
    {
        private string ModulePath
        {
            get { 

                
                string ModuleCSFilename = RulesCompiler.GetFileNameFromType(GetType());
                string ModuleBaseDirectory = Path.GetDirectoryName(ModuleCSFilename);

                return ModuleBaseDirectory;
            }
        }

        public CreaturePackRuntimePlugin(ReadOnlyTargetRules Target)
            : base(Target)
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
            PublicIncludePaths.AddRange(new string[] { ModulePath + "/Public", });
            PrivateIncludePaths.AddRange(new string[] { ModulePath + "/Private", ModulePath + "/Public",});

            PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

            PrivateDependencyModuleNames.AddRange(new string[] { "RHI", "RenderCore", "ShaderCore" });
        }
    }
}
