using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

using AutomationTool;

namespace UTVehicles.Automation
{
	class Packager : CommandUtils
	{
		// TODO: Read from config
		public static string URL_REPO_BASE = "https://github.com/UTCommunity/UTVehicles";

		public static string CopyFilesToZip(string ZipBaseFolder, string BaseDirectory, string InZipDir, FileFilter Filter, bool NoConversion, bool ParseTextOnly = false)
		{
			string TempFolder = GetRandomFolder(ZipBaseFolder);
			string CopyFolder = Path.Combine(TempFolder, InZipDir);

			CreateDirectory(CopyFolder);
			foreach (string FilteredFile in Filter.ApplyToDirectory(BaseDirectory, true))
			{
				string srcf = Path.Combine(BaseDirectory, FilteredFile);
				string destfo = Path.GetDirectoryName(FilteredFile);
				string destfi = Path.Combine(CopyFolder, FilteredFile);

				if (!NoConversion && Path.GetExtension(FilteredFile).Equals(".pod", StringComparison.InvariantCultureIgnoreCase))
				{
					var reader = new StreamReader(srcf);
					string content = reader.ReadToEnd();
					reader.Dispose();

					var parser = Parser.BaseParser.Create<Parser.POD.PODParser>(new StreamReader(srcf));

					if (ParseTextOnly)
					{
						using (var writer = new StreamWriter(destfi + ".txt"))
						{
							writer.WriteLine(parser.Text());
						}
					}
					else
					{
						using (var writer = new StreamWriter(destfi + ".htm"))
						{
							writer.WriteLine(parser.Flatten());
						}
					}
				}

				else if (!NoConversion && Path.GetExtension(FilteredFile).Equals(".md", StringComparison.InvariantCultureIgnoreCase))
				{
					var m = new MarkdownSharp.Markdown();

					var reader = new StreamReader(srcf);
					string content = reader.ReadToEnd();
					reader.Dispose();

					var UrlEvaluator = new MatchEvaluator(delegate(Match match)
					{
						string outstr = match.ToString();
						if (match.Groups.Count == 2)
						{
							string relfile = match.Groups[1].Value;

							string MailPattern = @"^(?("")("".+?(?<!\\)""@)|(([0-9a-z]((\.(?!\.))|[-!#\$%&'\*\+/=\?\^`\{\}\|~\w])*)(?<=[0-9a-z])@))";
							string DomainPattern = @"(?(\[)(\[(\d{1,3}\.){3}\d{1,3}\])|(([0-9a-z][-\w]*[0-9a-z]*\.)+[a-z0-9][\-a-z0-9]{0,22}[a-z0-9]))";

							if (!relfile.Contains("://") && !Regex.Match(relfile, MailPattern + DomainPattern).Success)
							{
								if (relfile.StartsWith("//"))
								{
									relfile = "https:" + relfile;
								}
								else if (relfile.StartsWith("/../../") || relfile.StartsWith("../../"))
								{
									relfile = relfile.Replace("../../", "");
									relfile = URL_REPO_BASE + "/" + relfile.TrimStart('/');
								}
								else
								{
									string[] splits = relfile.Split("?|#&".ToCharArray());
									if (Regex.Match(relfile, DomainPattern).Success && !File.Exists(Path.Combine(BaseDirectory, relfile)))
									{
										relfile = "https:" + relfile;
									}
									else if (splits.Length > 1)
									{
										if (splits[0].Length >= 260 || !File.Exists(Path.Combine(BaseDirectory, splits[0])))
										{
											relfile = URL_REPO_BASE + splits[0];
										}
									}
									else
									{
										string p = Path.Combine(BaseDirectory, relfile);
										if (!File.Exists(p) && !Directory.Exists(p))
										{
											relfile = URL_REPO_BASE + "/" + splits[0].TrimStart('/');
										}
									}
								}
							}

							outstr = match.Groups[0].Value.Replace(match.Groups[1].Value, relfile);
						}

						return outstr;
					});

					var FileEvaluator = new MatchEvaluator(delegate(Match match)
					{
						string outstr = match.ToString();
						if (match.Groups.Count == 6)
						{
							string seps = "?|#&";
							string filename = match.Groups[3].Value;
							string[] splits = filename.Split(seps.ToCharArray());
							if (splits.Length > 0 && File.Exists(Path.Combine(BaseDirectory, splits[0])))
							{
								string ext = match.Groups[4].Value;
								if (splits.Length > 1) ext = ext.Split(seps.ToCharArray())[0];
								filename = filename.Replace(splits[0], splits[0] + ".htm");
							}

							outstr = string.Format("[{0}{1}{2}{3}", match.Groups[1].Value, match.Groups[2].Value, filename, match.Groups[5].Value);
						}

						return outstr;
					});

					if (ParseTextOnly)
					{
						m.OnlyText = true;
						m.AutoHyperlink = false;
						content = m.Transform(content);

						using (var writer = new StreamWriter(destfi + ".txt"))
						{
							writer.WriteLine(content.Replace("\n", "\r\n"));
						}
					}
					else
					{
						// fix relative links
						content = Regex.Replace(content, @"\[.*?\]\((.*?)\)", UrlEvaluator);
						content = Regex.Replace(content, @"\[\s*[a-zA-Z0-9_-]+\s*\]\s*:\s*(\S+)\s*", UrlEvaluator);

						// redirect files in HTML
						content = Regex.Replace(content, @"\[(.*?)(\]\()(.*\.(pod|md)\S*)(\))", FileEvaluator, RegexOptions.IgnoreCase);
						content = Regex.Replace(content, @"\[(\s*[a-zA-Z0-9_-]+\s*)(\]\s*:\s*)(.*\.(pod|md)\S*)(\s*)", FileEvaluator);

						using (var writer = new StreamWriter(destfi + ".htm"))
						{
							writer.WriteLine(m.Transform(content));
						}
					}
				}
				else
				{
					CopyFile(srcf, destfi);
				}
			}

			return TempFolder;
		}

		public static string GetRandomFolder(string BaseDir)
		{
			string randomfolder = "";
			do
			{
				randomfolder = CombinePaths(BaseDir, Path.GetRandomFileName()).Replace(".", "");
			} while (DirectoryExists(randomfolder));

			return randomfolder;
		}
	}
}
