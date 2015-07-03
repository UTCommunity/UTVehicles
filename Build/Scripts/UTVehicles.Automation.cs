using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.IO;
using System.Linq;
using AutomationTool;
using UnrealBuildTool;
using System.Text.RegularExpressions;
using System.Net;
using System.Reflection;
using System.Web.Script.Serialization;
using EpicGames.MCP.Automation;
using EpicGames.MCP.Config;

public class PluginPackageZip : BuildCommand
{
    public string DLCName;
    public string DLCMaps;
    public string AssetRegistry;
    public string VersionString;

	public string PluginsDir;
	public string DLCDir;
	private string PublishDir;

    public override void ExecuteBuild()
	{
		DLCName = ParseParamValue("Plugin", "");
		DLCDir = CombinePaths(CmdEnv.LocalRoot, "UnrealTournament", "Plugins", DLCName);
		PluginsDir = CombinePaths(CmdEnv.LocalRoot, "UnrealTournament", "Plugins");
		PublishDir = CombinePaths(DLCDir, "Releases");

		// Right now all platform asset registries seem to be the exact same, this may change in the future
		AssetRegistry = ParseParamValue("ReleaseVersion", "UTVersion0");
		VersionString = ParseParamValue("Version", "NOVERSION");

		// Zip plugin
		FileFilter Filter = new FileFilter();
		Filter.ExcludeConfidentialFolders();
		Filter.ExcludeConfidentialPlatforms();

		// include bin and plugin content but only valid editor files
		Filter.Include("Binaries/...");
		Filter.Include("Config/...");
		Filter.Include("Content/.../*.uasset");
		Filter.Include("Content/.../*.umap");

		Filter.Include("Readme*");
		Filter.Include("*.txt");

		// ignore debug symbols
		Filter.Exclude("*.pdb");

		// Add source if required
		if (ParseParam("source"))
		{
			Filter.Include("Source/...");
			Filter.Include("Build/...");
		}
		Filter.Exclude("Build/.../Obj/...");
		Filter.Exclude("/Intermediate/...");

		// ignore git files
		Filter.Exclude(".git*");
		Filter.Exclude(".git/...");


		// create file name
		string ZipFileName = GenerateFileName() + ".zip";
		string ZipFile = CombinePaths(PublishDir, ZipFileName);
		string ZipDir = CombinePaths(CmdEnv.LocalRoot, "UnrealTournament", "Plugins", DLCName);

		// check if file exists and abort...
		if (FileExists(ZipFile) && !ParseParam("overwrite"))
		{
			throw new AutomationException("Release file {0} already exists.", ZipFileName);
		}

		ZipFilesEx(ZipFile, DLCName, ZipDir, Filter);
	}

	public string GenerateFileName()
	{
		string VersionNumber = ReadFromFile("VERSION", "DEV");
		string ChangeListNumber = ReadFromFile("CHANGELIST", "LATEST");
		string PreReleaseString = ReadFromFile("VERSION", "NIGHTLY", 1);
		string PreReleaseNumber = ReadFromFile("VERSION", "", 2);

		// find the build number based on main git repo and all its sub modules
		int buildnumber = 0;
		string metadata = "";
		if (DirectoryExists(DLCDir, ".git"))
		{
			string GitFolder = CombinePaths(DLCDir, ".git");

			int codecount = 0;
			using (var repo = new LibGit2Sharp.Repository(GitFolder))
			{
				codecount = repo.Commits.Count();
				buildnumber += codecount;
				var comm = repo.Commits.First();

				metadata = "+" + codecount + "." + comm.Id.ToString(7);

				foreach (var submodule in repo.Submodules)
				{
					string SubFolder = CombinePaths(DLCDir, submodule.Path, ".git");
					using (var subrepo = new LibGit2Sharp.Repository(SubFolder))
					{
						int subcount = subrepo.Commits.Count();
						metadata += "-" + subcount + "." + submodule.HeadCommitId.ToString(7);
						buildnumber += subcount;
					}
				}
			}
		}

		string PackageType = "";
		if (ParseParam("package"))
		{
			PackageType = "-" + ParseParamValue("package");
		}

		// set pre-release tag
		string PreRelease = "";
		if (PreReleaseString.Length > 0)
		{
			PreRelease = "-" + PreReleaseString;
		}

		// try to find pre-release number
		string BaseFileName = DLCName + PackageType + "_" + "v" + VersionNumber;
		if (DirectoryExists(PublishDir))
		{
			string[] files = FindFiles(BaseFileName + PreRelease + ".", false, PublishDir);
			PreRelease += "." + Math.Max(1, files.Length);
			PreRelease += "-" + buildnumber;
		}

		string Platforms = "";
		var Params = MakeUTDLC.GetParams(this, DLCName, AssetRegistry);
		List<DeploymentContext> DeployContextServerList = MakeUTDLC.CreateDeploymentContext(Params, false, true);
		if (DeployContextServerList.Count > 0)
		{
			List<string> PlatformsList = new List<string>();
			foreach (DeploymentContext SC in DeployContextServerList)
			{
				PlatformsList.Add(SC.CookSourcePlatform.PlatformType.ToString());
				//Platforms = SC
			}

			Platforms = String.Join("", PlatformsList);
		}

		return BaseFileName + PreRelease + metadata + "_" + Platforms + "-" + ChangeListNumber;
	}

	/// <summary>
	/// Creates a zip file containing the given input files
	/// </summary>
	/// <param name="ZipFileName">Filename for the zip</param>
	/// <param name="Filter">Filter which selects files to be included in the zip</param>
	/// <param name="BaseDirectory">Base directory to store relative paths in the zip file to</param>
	public static void ZipFilesEx(string ZipFileName, string ZipBaseFolder, string BaseDirectory, FileFilter Filter)
	{
		Ionic.Zip.ZipFile Zip = new Ionic.Zip.ZipFile();
		Zip.UseZip64WhenSaving = Ionic.Zip.Zip64Option.Always;
		foreach (string FilteredFile in Filter.ApplyToDirectory(BaseDirectory, true))
		{
			Zip.AddFile(Path.Combine(BaseDirectory, FilteredFile), Path.Combine(ZipBaseFolder, Path.GetDirectoryName(FilteredFile)));
		}
		CreateDirectory(Path.GetDirectoryName(ZipFileName));
		Zip.Save(ZipFileName);
	}

	public string ReadFromFile(string FileName, string Default = null, int Level = 0)
	{
		string FilePath = CombinePaths(DLCDir, FileName);
		if (File.Exists(FilePath))
		{
			using (StreamReader Reader = new StreamReader(FilePath))
			{
				int CurrentLevel = 0;
				while (Reader.Peek() >= 0)
				{
					string LineRead = Reader.ReadLine();
					if (CurrentLevel == Level)
					{
						int ind = LineRead.IndexOf(" ");
						if (ind > 0) LineRead = LineRead.Substring(0, ind);
						if (LineRead.Length > 0)
						{
							return LineRead;
						}
					}
					CurrentLevel += 1;
				}
			}
		}

		return Default;
	}
}
