using System;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;
using System.Linq;

using AutomationTool;
using UTVehicles.Automation.Parser;

namespace UTVehicles.Automation
{
	public class UTVehicles_PackageZip : BuildCommand
	{
		public static string PLUGIN_NAME = "UTVehicles";
		public static string VERSION_NAME = "VERSION";

		public static string BUILD_PREFIX = "Build-";
		public static string DEFAULT_PACKAGE_NAME = "{PluginName}_v{VERSION}{PRERELEASE}{META}{PUBLISH}{SOLUTION}_{CHANGELIST}-{PLATFORMS}";

		public static string COMMIT_TITLE_VERSION_PUBLISH = "New published version: {0}";
		public static string COMMIT_TITLE_VERSION_INTERNAL = "New internal build: {0}";

		public static string GIT_IGNORE_FILE = ".gitignore";
		public static string GIT_FOLDER_NAME = ".git";

		public string DLCName;
		public string DLCMaps;
		public string AssetRegistry;
		public string VersionString;

		public string PluginsDir;
		public string DLCDir;
		private string PublishDir;

		public string FileVersionName;

		public MetaInfo BuildMeta;
		public VersionInfo BuildVersion;

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
				return CodeRepo.CommitCount + (SubRepos.Count > 0 ? "." + String.Join(".", String.Concat(SubRepos.Select(s => s.CommitCount.ToString()))) : "");
			}

			public string ToExtendedString()
			{
				return string.Format("Build: {0} SHA: {1} Commit: {2}{3}", BuildNumber, CodeRepo.SHAShort, CodeRepo.CommitCount, 
					SubRepos.Count > 0 ? " + " + String.Join(" + ", String.Concat(SubRepos.Select(s => string.Format("{0} SHA: {1} Commits: {2}", s.Name, s.SHAShort, s.CommitCount)))) : ""
				);
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
				return IsValid() ? Version + "-" + PreRelease : "";
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

			// prevent processing automation process for the wrong plugin (if mutliple PluginPackageZip commands exist)
			if (!DLCName.Equals(PLUGIN_NAME, StringComparison.InvariantCultureIgnoreCase))
			{
				// no Expception as other plugins need to be processed
				return;
			}

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
				string GitFolder = CombinePaths(DLCDir, GIT_FOLDER_NAME);
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

			//
			// Create filter list to exclude folder/files from zip
			//


			FileFilter Filter = new FileFilter();
			Filter.ExcludeConfidentialFolders();
			Filter.ExcludeConfidentialPlatforms();

			// read git ignore file and add these to the filder
			var FilterInclude = new List<string>();
			var FilterExclude = new List<string>();
			{
				string GitFolder = CombinePaths(DLCDir, GIT_FOLDER_NAME);
				if (DirectoryExists(GitFolder))
				{
					using (var repo = new LibGit2Sharp.Repository(GitFolder))
					{
						var subrepos = new List<LibGit2Sharp.Repository>();
						foreach (LibGit2Sharp.Submodule submodule in repo.Submodules)
						{
							string SubFolder = CombinePaths(DLCDir, submodule.Path);
							if (DirectoryExists(SubFolder))
							{
								string relpath = SubFolder.Substring(DLCDir.Length).Replace("\\", "/").Trim('/');
								FilterExclude.Add("/" + relpath + "/**");
								subrepos.Add(new LibGit2Sharp.Repository(SubFolder));
							}
						}

						foreach (var file in FindFiles("*", true, DLCDir))
						{
							bool subuntracked = false;
							foreach (var subrepo in subrepos)
							{
								if (file.StartsWith(subrepo.Info.WorkingDirectory))
								{
									LibGit2Sharp.FileStatus substatus = subrepo.RetrieveStatus(file);
									if (substatus == LibGit2Sharp.FileStatus.Untracked)
									{
										subuntracked = true;
										string relpath = file.Substring(DLCDir.Length).Replace("\\", "/").TrimStart('/');
										FilterInclude.Add(relpath);
										break;
									}
								}
							}

							if (!subuntracked)
							{
								LibGit2Sharp.FileStatus status = repo.RetrieveStatus(file);

								if (status == LibGit2Sharp.FileStatus.Untracked)
								{
									string relpath = file.Substring(DLCDir.Length).Replace("\\", "/").TrimStart('/');
									FilterInclude.Add(relpath);
								}
							}
						}

						foreach (var subrepo in subrepos)
						{
							subrepo.Dispose();
						}
						subrepos.Clear();
					}
				}

				string GitIgnoreFile = CombinePaths(DLCDir, GIT_IGNORE_FILE);
				if (FileExists(GitIgnoreFile))
				{
					//HasGitIgnore = true;
					var GitIgnores = GitIgnoreParser.ReadFromFile(GitIgnoreFile);

					foreach (var entry in GitIgnores.Entries)
					{
						string pattern = entry.Pattern;
						if (entry.Pattern.EndsWith("/")) pattern += "**";
						if (entry.Type == GitIgnoreParser.GitIgnoreTypInfo.Include)
						{
							FilterInclude.Add(pattern);
						}
						else if (entry.Type == GitIgnoreParser.GitIgnoreTypInfo.Exclude)
						{
							FilterExclude.Add(pattern);
						}
					}
				}
			}


			// TODO: Add build file for includes

			// include bin and plugin content but only valid editor files
			foreach (var f in FilterExclude)
			{
				Filter.Include(f.Replace("**", "..."));
			}

			if (FilterInclude.Count == 0)
			{
				Filter.Include("Readme*");
				Filter.Include("*.txt");
				Filter.Include("*.md");
				Filter.Include("*.pod");
			}
			else
			{
				Filter.Include("/*.*");
			}

			// Add source if required
			if (ParseParam("source"))
			{
				Filter.Include("/Source/...");
				Filter.Include("/Build/...");
				Filter.Exclude("/Build/.../Obj/...");
			}
			else
			{
				Filter.Exclude("/Build/...");
			}

			foreach (var f in FilterInclude)
			{
				Filter.Exclude(f.Replace("**", "..."));
			}

			Filter.Include("/Binaries/...");

			// ignore debug symbols
			Filter.Exclude("/Binaries/.../*.pdb");

			// ignore git files
			Filter.Exclude(".git*");
			Filter.Exclude(".git/...");

			// always include plugin descriptor file
			Filter.Include("*.uplugin");

			// exclude releaes foldre
			Filter.Exclude("/Releases/...");
			Filter.Exclude("/Releases/.../...");


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

			string zipfolder = Packager.CopyFilesToZip(PublishDir, ZipDir, DLCName, Filter, NoConversion: HasParam("source"), ParseTextOnly: HasParam("ParseTextOnly"));
			ZipFiles(ZipFile, zipfolder, new FileFilter(FileFilterType.Include));

			if (ParseParam("publish") && BuildVersion.IsValid())
			{
				bool IsInternal = false;
				if (ParseParam("release"))
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

				string GitFolder = CombinePaths(DLCDir, GIT_FOLDER_NAME);
				if (ParseParam("commit") && DirectoryExists(GitFolder))
				{
					using (var repo = new LibGit2Sharp.Repository(GitFolder))
					{
						string commmessage;
						if (IsInternal)
							commmessage = string.Format(COMMIT_TITLE_VERSION_INTERNAL, BuildVersion.ToString());
						else
							commmessage = string.Format(COMMIT_TITLE_VERSION_PUBLISH, BuildVersion.ToShortVersion());

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

						// Commit new tag
						LibGit2Sharp.Tag t = repo.Tags.Add(string.Format("v{0}", BuildVersion.ToShortVersion()), repo.Head.Tip);
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
			string GitFolder = CombinePaths(DLCDir, GIT_FOLDER_NAME);
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
			if (HasParam("publish") && HasParam("release"))
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
				if (!HasParam("release))
				{
					if (BuildMeta.BuildNumber > 0) PreRelease += "." + (BuildVersion.BuildAsNumber + 1);
					PreRelease += "-" + BuildMeta.Build;
				}
			}

			// Create list of platforms
			string Platforms = "";
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

			// Check if the current internal/local build already exists
			if (!ParseParam("release"))
			{
				var formatargs_check = (from x in formatargs select x).ToDictionary(x => x.Key, x => x.Value);
				formatargs_check["PreRelease"] = "*";

				string BaseFileName = StringFormat(filename, formatargs_check);
				string[] files = FindFiles(BaseFileName + ".*", false, PublishDir);
				if (files.Length > 0)
				{
					throw new AutomationException("Internal/local build already exists. Nothing's changed. Same build numer.");
				}
			}

			return StringFormat(filename, formatargs);
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
			Regex regEx = new Regex(oldValue, RegexOptions.IgnoreCase | RegexOptions.Multiline);
			return regEx.Replace(originalString, newValue);
		}
	}
}
