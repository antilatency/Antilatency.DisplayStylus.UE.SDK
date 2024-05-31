using System;
using System.IO;

namespace UnrealBuildTool.Rules {
    public class AntilatencyDisplayStylus : ModuleRules {

        public AntilatencyDisplayStylus(ReadOnlyTargetRules Target) : base(Target) {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

#if UE_4_27_OR_LATER
            bUsePrecompiled = false;
#endif

            PublicIncludePathModuleNames.AddRange(
                new string[] {
                    "Launch",
                    "Core"
                }
            );

            PublicDependencyModuleNames.AddRange(
                new string[] {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "Projects",
                    "ProceduralMeshComponent",
					"AntilatencyDeviceNetwork",
					"AntilatencyHardwareExtensionInterface",
                    "AntilatencyAltTracking",
                    "AntilatencyAltEnvironmentSelector",
                    "AntilatencyAltEnvironmentRectangle",
                    "AntilatencyPhysicalConfigurableEnvironment"
                }
            );
        }
    }
}
