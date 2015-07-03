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

	public string FileVersionName;
	public string GitFolderName = ".git";

	public MetaInfo BuildMeta;
	public VersionInfo BuildVersion;

	public static string VERSION_NAME = "VERSION";

	public static string BUILD_PREFIX = "Build-";
	public static string DEFAULT_PACKAGE_NAME = "{PluginName}_v{VERSION}{PRERELEASE}{META}{PUBLISH}{SOLUTION}_{CHANGELIST}-{PLATFORMS}";

	public static string COMMIT_TITLE_VERSION_PUBLISH = "New published version: {0}";
	public static string COMMIT_TITLE_VERSION_INTERNAL = "New internal build: {0}";
	
	public struct MetaInfo
	{
		public int BuildNumber;
		public string Build
		{
			get { return BuildNumber > 0 ? BuildNumber.ToString() : "UNKNOWN"; }
		}

		public MetaGitInfo CodeRepo;
		public List<MetaGitInfo> SubRepos;

		public MetaInfo(int build)
		{
			this.BuildNumber = build;
			this.CodeRepo = new MetaGitInfo();
			this.SubRepos = new List<MetaGitInfo>();
		}

		public override string ToString()
		{
			return "+" + CodeRepo.ToString() + String.Concat(SubRepos);
		}

		public string ToShortString()
		{
			return CodeRepo.CommitCount + (SubRepos.Count > 0 ? "." + String.Join(".", String.Concat(SubRepos.Select(s=>s.CommitCount.ToString()))) : "");
		}

		public string ToExtendedString()
		{
			// TODO: extend meta string
			return ToString();
		}
	}

	public struct MetaGitInfo
	{
		public string Name;

		public int CommitCount;
		public string SHA;
		public string SHAShort;

		public override string ToString()
		{
			return (Name != "." ? "-" : "") + CommitCount + "." + SHAShort;
		}
	}

	public struct VersionInfo
	{
		public string Version;
		public string PreRelease;
		public int BuildAsNumber;

		public string Build
		{
			get { return BuildAsNumber.ToString(); }
			set
			{
				try { BuildAsNumber = int.Parse(value); }
				catch { }
			}
		}

		public bool IsValid()
		{
			if (Version == null || PreRelease == null)
				return false;

			return Version.Length > 0 || PreRelease.Length > 0;
		}

		public bool IsValid(bool BuildCheck)
		{
			return IsValid() && (!BuildCheck || BuildAsNumber > 0);
		}

		public void FromData(List<string> data)
		{
			if (data.Count > 0) Version = data[0];
			if (data.Count > 1) PreRelease = data[1];
			if (data.Count > 2) Build = data[2];
		}

		public string[] ToData()
		{
			return ToData(false);
		}

		public string[] ToData(bool NoBuild)
		{
			List<string> data = new List<string>();
			data.Add(Version);
			data.Add(PreRelease);
			if (!NoBuild) data.Add(BuildAsNumber.ToString());
			return data.ToArray();
		}

		public string ToShortVersion()
		{
			return IsValid() ? Version + "-" +PreRelease : "";
		}

		public override string ToString()
		{
			return IsValid() ? ToShortVersion() + (BuildAsNumber > 0 ? "." + BuildAsNumber.ToString() : "") : "";
		}
	}

    public override void ExecuteBuild()
	{
		DLCName = ParseParamValue("Plugin", "");
		DLCDir = CombinePaths(CmdEnv.LocalRoot, "UnrealTournament", "Plugins", DLCName);
		PluginsDir = CombinePaths(CmdEnv.LocalRoot, "UnrealTournament", "Plugins");
		PublishDir = CombinePaths(DLCDir, "Releases");

		// Right now all platform asset registries seem to be the exact same, this may change in the future
		AssetRegistry = ParseParamValue("ReleaseVersion", "UTVersion0");
		VersionString = ParseParamValue("Version", "NOVERSION");

		FileVersionName = VERSION_NAME;

		// Check parms
		if (!DirectoryExists(DLCDir))
		{
			throw new AutomationException("Plugin folder '{0}' does not exist. Verify that '-plugin={1}' is set up properly.", DLCDir, DLCName);
		}
		if (new DirectoryInfo(PluginsDir).FullName.Equals(new DirectoryInfo(DLCDir).FullName, StringComparison.InvariantCultureIgnoreCase))
		{
			throw new AutomationException("No Plugin given. Verify that '-plugin=PLUGINNAME' is set.");
		}

		// Check for changed files in the git repo and abort
		if (!ParseParam("NoGitCheck"))
		{
			string GitFolder = CombinePaths(DLCDir, GitFolderName);
			if (DirectoryExists(GitFolder))
			{
				using (var repo = new LibGit2Sharp.Repository(GitFolder))
				{
					LibGit2Sharp.StatusOptions check = new LibGit2Sharp.StatusOptions();
					check.ExcludeSubmodules = true;

					LibGit2Sharp.RepositoryStatus status = repo.RetrieveStatus(check);
					if (status.IsDirty)
					{
						throw new AutomationException("Git code repository has uncommited changes.");
					}

					foreach (LibGit2Sharp.Submodule submodule in repo.Submodules)
					{
						LibGit2Sharp.SubmoduleStatus substatus = submodule.RetrieveStatus();
						if (!substatus.HasFlag(LibGit2Sharp.SubmoduleStatus.Unmodified))
						{
							throw new AutomationException("Git sub module '{0}' has uncommited changes.", submodule.Name);
						}
					}
				}
			}
		}

		// Check if binaries exist
		{
			string BinariesFolder = CombinePaths(DLCDir, "Binaries");
			if (!DirectoryExists(BinariesFolder))
			{
				throw new AutomationException("No Binaries folder. Please, recompile the plugin from source files");
			}

			// TODO: Check for specific files (SO for linux and DLL for windows)
			if (FindFiles("*", true, BinariesFolder).Length == 0)
			{
				throw new AutomationException("No binary files. Please, recompile the plugin from source files");
			}
		}

		// Zip plugin
		FileFilter Filter = new FileFilter();
		Filter.ExcludeConfidentialFolders();
		Filter.ExcludeConfidentialPlatforms();

		// TODO: use .gitignore to filter files
		// TODO: Add build file for includes

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
		else
		{
			Filter.Exclude("Build/...");
		}
		Filter.Exclude("Build/.../Obj/...");
		Filter.Exclude("/Intermediate/...");

		// ignore git files
		Filter.Exclude(".git*");
		Filter.Exclude(".git/...");

		// always include plugin descriptor file
		Filter.Include("*.uplugin");


		// read current version info from file
		CreateVersionInfo();

		// create meta info which is stored in BuildMeta
		CreateMetaInfo();

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

		if (BuildVersion.IsValid())
		{
			bool IsInternal = false;
			if (ParseParam("publish") )
			{
				// remove internal build (public release) ... just in case
				DataWriteToFile(FileVersionName, BuildVersion.ToData(true));
			}
			else
			{
				IsInternal = true;

				// increase internal build
				BuildVersion.BuildAsNumber += 1;
				DataWriteToFile(FileVersionName, BuildVersion.ToData());
			}

			string GitFolder = CombinePaths(DLCDir, GitFolderName);
			if (ParseParam("commit") && DirectoryExists(GitFolder))
			{
				using (var repo = new LibGit2Sharp.Repository(GitFolder))
				{
					string commmessage;
					if (IsInternal)
						commmessage = string.Format(COMMIT_TITLE_VERSION_PUBLISH, BuildVersion.ToString());
					else
						commmessage = string.Format(COMMIT_TITLE_VERSION_INTERNAL, BuildVersion.ToShortVersion());

					commmessage += Environment.NewLine;
					commmessage += Environment.NewLine;

					commmessage += BuildMeta.ToExtendedString();

					// Stage the file
					LibGit2Sharp.StageOptions stageoptions = new LibGit2Sharp.StageOptions();
					repo.Stage(FileVersionName, stageoptions);

					// Create the committer's signature and commit
					LibGit2Sharp.Signature author = repo.Config.BuildSignature(DateTime.Now);
					LibGit2Sharp.Signature committer = author;

					// Commit to the repository
					LibGit2Sharp.CommitOptions options = new LibGit2Sharp.CommitOptions();
					LibGit2Sharp.Commit commit = repo.Commit(commmessage, author, committer, options);

					// TODO: Commit new tag
					//LibGit2Sharp.Tag t = repo.Tags.Add(version.ToShortVersion(), repo.Head.Tip, author, version.ToString());
					//Console.WriteLine(t.ToString());
				}
			}
		}
	}

	public void CreateVersionInfo()
	{
		BuildVersion = new VersionInfo();
		BuildVersion.FromData(DataReadFromFile(FileVersionName));
	}

	public void CreateMetaInfo()
	{
		BuildMeta = new MetaInfo(0);

		// find the build number based on main git repo and all its sub modules
		int buildnumber = 0;
		string GitFolder = CombinePaths(DLCDir, GitFolderName);
		if (DirectoryExists(GitFolder))
		{
			using (var repo = new LibGit2Sharp.Repository(GitFolder))
			{
				MetaGitInfo gitinfo = new MetaGitInfo();
				gitinfo.Name = ".";
				gitinfo.CommitCount = repo.Commits.Count();
				gitinfo.SHA = repo.Commits.First().Id.ToString();
				gitinfo.SHAShort = repo.Commits.First().Id.ToString(7);

				buildnumber += gitinfo.CommitCount;
				BuildMeta.CodeRepo = gitinfo;

				foreach (var submodule in repo.Submodules)
				{
					string SubFolder = CombinePaths(DLCDir, submodule.Path);
					if (DirectoryExists(SubFolder))
					{
						using (var subrepo = new LibGit2Sharp.Repository(SubFolder))
						{
							MetaGitInfo subgitinfo = new MetaGitInfo();
							subgitinfo.Name = submodule.Name;
							subgitinfo.CommitCount = subrepo.Commits.Count();
							subgitinfo.SHA = submodule.HeadCommitId.ToString();
							subgitinfo.SHAShort = submodule.HeadCommitId.ToString(7);

							buildnumber += subgitinfo.CommitCount;
							BuildMeta.SubRepos.Add(subgitinfo);
						}
					}
				}
			}
		}

		BuildMeta.BuildNumber = buildnumber;
	}

	public string GenerateFileName()
	{
		string VersionNumber = BuildVersion.IsValid() ? BuildVersion.Version : DataReadFromFile(FileVersionName, "DEV");
		string ChangeListNumber = DataReadFromFile("CHANGELIST", "LATEST");
		string PreReleaseString = BuildVersion.IsValid() ? BuildVersion.PreRelease : DataReadFromFile(FileVersionName, "NIGHTLY", 1);
		string PreReleaseNumber = BuildVersion.Build;
		string PublishString = "";

		// set meta string
		string metadata = BuildMeta.ToString();
		if (HasParam("publish"))
		{
			metadata = "";
			PublishString = "_" + BUILD_PREFIX + BuildMeta.ToShortString();
		}

		// set configuration string (Editor/Game/Full/...)
		string SolutionString = ParseParamValue("solution", "");
		if (HasParam("solution"))
		{
			SolutionString = "_" + ParseParamValue("solution", "");
		}

		// set pre-release tag
		string PreRelease = "";
		if (PreReleaseString.Length > 0)
		{
			PreRelease = "-" + PreReleaseString;
			if (!HasParam("publish"))
			{
				if (BuildMeta.BuildNumber > 0) PreRelease += "." + (BuildVersion.BuildAsNumber + 1);
				PreRelease += "-" + BuildMeta.Build;
			}
		}

		string Platforms = "";
		// Create list of platforms
		{
			var Params = MakeUTDLC.GetParams(this, DLCName, AssetRegistry);
			List<DeploymentContext> DeployContextServerList = MakeUTDLC.CreateDeploymentContext(Params, false, true);
			Platforms = String.Join("-", String.Concat(DeployContextServerList.Select(sc => sc.CookSourcePlatform.PlatformType.ToString())));
		}

		string filename = DataReadFromFile(".package.name", DEFAULT_PACKAGE_NAME);
		Dictionary<string, object> formatargs = new Dictionary<string, object>
		{
			{"PluginName", DLCName },
			{"Version", VersionNumber},
			{"PreRelease", PreRelease},
			{"Solution", SolutionString},
			{"Meta", metadata},
			{"ChangeList", ChangeListNumber},
			{"Platforms", Platforms},
			{"Publish", PublishString},
		};

		// Check if the current internal build already exists
		if (!ParseParam("publish"))
		{
			var formatargs_check = (from x in formatargs select x).ToDictionary(x => x.Key, x => x.Value);
			formatargs_check["PreRelease"] = "*";

			string BaseFileName = StringFormat(filename, formatargs_check);
			string[] files = FindFiles(BaseFileName + ".*", false, PublishDir);
			if (files.Length > 0)
			{
				throw new AutomationException("Internal build already exists. Nothing's changed. Same build numer.");
			}
		}

		return StringFormat(filename, formatargs);
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

	public string DataReadFromFile(string FileName, string Default = null, int Level = 0)
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

	public List<string> DataReadFromFile(string FileName)
	{
		List<string> Lines = new List<string>();
		string FilePath = CombinePaths(DLCDir, FileName);
		if (File.Exists(FilePath))
		{
			using (StreamReader Reader = new StreamReader(FilePath))
			{
				while (Reader.Peek() >= 0)
				{
					Lines.Add(Reader.ReadLine());
				}
			}
		}

		return Lines;
	}

	public void DataWriteToFile(string FileName, string NewValue, int Level = 0)
	{
		string FilePath = CombinePaths(DLCDir, FileName);
		if (File.Exists(FilePath))
		{
			List<string> s = new List<string>();
			using (StreamReader Reader = new StreamReader(FilePath))
			{
				int CurrentLevel = 0;
				while (Reader.Peek() >= 0)
				{
					string LineRead = Reader.ReadLine();
					s.Add(CurrentLevel == Level ? LineRead : NewValue);
					CurrentLevel += 1;
				}

				DataWriteToFile(FilePath, s.ToArray());
			}
		}
	}

	public void DataWriteToFile(string FileName, params string[] NewValues)
	{
		string FilePath = CombinePaths(DLCDir, FileName);
		using (StreamWriter Writer = new StreamWriter(FilePath))
		{
			foreach (string NewValue in NewValues)
			{
				Writer.WriteLine(NewValue);
			}
		}
	}

	public void DataDeleteFile(string FileName)
	{
		string FilePath = CombinePaths(DLCDir, FileName);
		DeleteFile(FilePath);
	}

	/// <summary>
	/// Checks if the given parameter is set
	/// </summary>
	/// <param name="ArgList">Argument list.</param>
	/// <param name="Param">Param to read its value.</param>
	/// <returns>True if param was found, false otherwise.</returns>
	public static bool HasParam(object[] ArgList, string Param)
	{
		bool Append = true;
		if (Param.EndsWith("="))
		{
			Append = false;
		}

		foreach (object Arg in ArgList)
		{
			string ArgStr = Arg.ToString();
			string ParamStr = Param + (Append ? "=" : "");
			if (ArgStr.Equals(Param, StringComparison.InvariantCultureIgnoreCase))
			{
				return true;
			}
			else if (ArgStr.StartsWith(ParamStr, StringComparison.InvariantCultureIgnoreCase))
			{
				return !Append || !ArgStr.EndsWith(ParamStr, StringComparison.InvariantCultureIgnoreCase);
			}
		}

		return false;
	}

	/// <summary>
	/// Checks if the given parameter is set
	/// </summary>
	/// <param name="Param">Param to read its value.</param>
	/// <returns>True if param was found, false otherwise.</returns>
	public bool HasParam(string Param)
	{
		return HasParam(Params, Param);
	}

	public static string StringFormat(string input, object p)
	{
		Dictionary<string, object> dic = new Dictionary<string, object>();
		System.ComponentModel.PropertyDescriptorCollection props = System.ComponentModel.TypeDescriptor.GetProperties(p);

		foreach (System.ComponentModel.PropertyDescriptor prop in props)
			dic.Add(prop.Name, prop.GetValue(p));

		return StringFormat(input, dic);
	}

	public static string StringFormat(string input, Dictionary<string, object> dic)
	{
		foreach (var d in dic)
			input = CaseInsenstiveReplace(input, "{" + d.Key + "}", (d.Value ?? "").ToString());

		return CaseInsenstiveReplace(input, "{(.+?)}", "");
	}

	/// <summary>
	/// A case insenstive replace function.
	/// </summary>
	/// <param name="originalString">The string to examine.</param>
	/// <param name="oldValue">The value to replace.</param>
	/// <param name="newValue">The new value to be inserted</param>
	/// <returns>A string</returns>
	public static string CaseInsenstiveReplace(string originalString, string oldValue, string newValue)
	{
		Regex regEx = new Regex(oldValue,
		   RegexOptions.IgnoreCase | RegexOptions.Multiline);
		return regEx.Replace(originalString, newValue);
	}
}
