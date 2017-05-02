using System.IO;

namespace UnrealBuildTool.Rules
{
    public class CreaturePlugin : ModuleRules
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
            get { return Path.GetFullPath(Path.Combine(ModulePath, "../ThirdParty/")); }
        }

        public bool LoadCreatureLib(TargetInfo Target)
        {
            Definitions.Add("GLM_FORCE_RADIANS");
            Definitions.Add("CREATURE_NO_USE_ZIP");
            Definitions.Add("CREATURE_NO_USE_EXCEPTIONS");
            Definitions.Add("CREATURE_MULTICORE");
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "Includes"));

            return true;
        }

        public CreaturePlugin(TargetInfo Target)
        {
            PublicIncludePaths.AddRange(new string[] { "CreaturePlugin/Public", });
            PrivateIncludePaths.AddRange(new string[] { "CreaturePlugin/Private", });

            PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

            PrivateDependencyModuleNames.AddRange(new string[] { "RHI", "RenderCore", "ShaderCore", "Json", "JsonUtilities" });

            LoadCreatureLib(Target);
        }
    }
}
