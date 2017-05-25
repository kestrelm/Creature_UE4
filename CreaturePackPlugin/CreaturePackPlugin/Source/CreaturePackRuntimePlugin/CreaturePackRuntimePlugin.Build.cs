using System.IO;

namespace UnrealBuildTool.Rules
{
    public class CreaturePackRuntimePlugin : ModuleRules
    {
        public CreaturePackRuntimePlugin(ReadOnlyTargetRules Target)
            : base(Target)
        {
            PublicIncludePaths.AddRange(new string[] { "CreaturePackRuntimePlugin/Public", });
            PrivateIncludePaths.AddRange(new string[] { "CreaturePackRuntimePlugin/Private", "CreaturePackRuntimePlugin/Public",});

            PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

            PrivateDependencyModuleNames.AddRange(new string[] { "RHI", "RenderCore", "ShaderCore" });
        }
    }
}
