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

	public string DLCDir;

    public override void ExecuteBuild()
    {
		DLCName = ParseParamValue("DLCName", "PeteGameMode");
		DLCDir = CombinePaths(CmdEnv.LocalRoot, "UnrealTournament", "Plugins", DLCName);

        VersionString = ParseParamValue("Version", "NOVERSION");

		string VersionNumber = ReadFromFile("VERSION", "DEV");
		string ChangeListNumber = ReadFromFile("CHANGELIST", "LATEST"); ;

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

		// ignore git files
		Filter.Exclude(".git");
		Filter.Exclude(".git/...");

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

		// create file name
		string PublishDir = CombinePaths(DLCDir, "Releases");
		string ZipFileName = DLCName + "_" + "v" + VersionNumber + "_" + ChangeListNumber + ".zip";
		string ZipFile = CombinePaths(PublishDir, ZipFileName);
		string ZipDir = CombinePaths(CmdEnv.LocalRoot, "UnrealTournament", "Plugins", DLCName);

		// check if file exists and abort...
		if (FileExists(new string[] { ZipFile }) && !ParseParam("overwrite"))
		{
			throw new AutomationException("Release file {0} already exists.", ZipFileName);
		}

		ZipFilesEx(ZipFile, DLCName, ZipDir, Filter);
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

	public string ReadFromFile(string FileName, string Default = null)
	{
		string FilePath = CombinePaths(DLCDir, FileName);
		if (File.Exists(FilePath))
		{
			using (StreamReader Reader = new StreamReader(FilePath))
			{
				string LineRead = Reader.ReadLine();
				int ind = LineRead.IndexOf(" ");
				if (ind > 0) LineRead = LineRead.Substring(0, ind);
				if (LineRead.Length > 0)
				{
					return LineRead;
				}
			}
		}

		return Default;
	}
}
