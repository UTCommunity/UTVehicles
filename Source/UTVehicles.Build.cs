namespace UnrealBuildTool.Rules
{
	public class UTVehicles : ModuleRules
	{
		public UTVehicles(TargetInfo Target)
		{
			PrivateIncludePaths.Add("Private");

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"UnrealTournament",
					"InputCore",
					"SlateCore",
				}
			);
		}
	}
}
