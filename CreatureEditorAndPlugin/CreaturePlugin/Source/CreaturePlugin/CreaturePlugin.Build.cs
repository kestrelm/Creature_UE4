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
            bool isLibrarySupported = false;

            if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
            {
                isLibrarySupported = true;

                string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x86";
                string LibrariesPath = Path.Combine(ThirdPartyPath, "CreatureLib", "Libraries");

                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "CreatureLib." + PlatformString + ".lib"));
            }
            else if (Target.Platform == UnrealTargetPlatform.Mac)
            {
                isLibrarySupported = true;
                string LibrariesPath = Path.Combine(ThirdPartyPath, "CreatureLib", "Libraries");
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libCreatureUE4Core.a"));

                Definitions.Add(" GLM_FORCE_RADIANS");
            }
            else if (Target.Platform == UnrealTargetPlatform.HTML5)
            {
                isLibrarySupported = true;
                string LibrariesPath = Path.Combine(ThirdPartyPath, "CreatureLib", "Libraries");
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "CreatureHTML5.bc"));

                Definitions.Add(" GLM_FORCE_RADIANS");
            }

            if (isLibrarySupported)
            {
                // Include path
                PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "CreatureLib", "Includes"));
            }

            Definitions.Add(string.Format("WITH_CREATURE_LIB_BINDING={0}", isLibrarySupported ? 1 : 0));

            return isLibrarySupported;
        }

        public CreaturePlugin(TargetInfo Target)
        {
            PublicIncludePaths.AddRange(new string[] { "CreaturePlugin/Public", });
            PrivateIncludePaths.AddRange(new string[] { "CreaturePlugin/Private", });

            PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

            PrivateDependencyModuleNames.AddRange(new string[] { "RHI", "RenderCore", "ShaderCore" });

            LoadCreatureLib(Target);
        }
    }
}
