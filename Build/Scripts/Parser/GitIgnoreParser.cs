using System;
using System.Collections.Generic;
using System.IO;

namespace UTVehicles.Automation.Parser
{
	public class GitIgnoreParser : BaseParser
	{
		public enum GitIgnoreTypInfo { Include, Exclude };

		public struct GitIgnoreInfo
		{
			public GitIgnoreTypInfo Type;
			public string Pattern;

			public override string ToString()
			{
				return string.Format("{0} {1}", Type == GitIgnoreTypInfo.Include ? "+" : "-", Pattern);
			}
		}

		private readonly List<GitIgnoreInfo> _Entries;
		public List<GitIgnoreInfo> Entries
		{
			get { return _Entries; }
		}

		private GitIgnoreParser()
		{
			_Entries = new List<GitIgnoreInfo>();
		}

		public GitIgnoreParser(string content)
			: this()
		{
			Parse(content);
		}

		public GitIgnoreParser(string[] lines)
			: this()
		{
			Parse(lines);
		}

		protected override void Parse(string[] lines)
		{
			foreach (string line in lines)
			{
				string trimmed = line.Trim();
				if (trimmed.StartsWith("#")) continue;
				if (trimmed.Length == 0) continue;

				var item = new GitIgnoreInfo();
				item.Type = trimmed.StartsWith("!") ? GitIgnoreTypInfo.Exclude : GitIgnoreTypInfo.Include;

				trimmed = line.TrimStart('!');
				trimmed = trimmed.Replace("\\", "/");
				trimmed = trimmed.Replace("//", "/");

				item.Pattern = trimmed;

				_Entries.Add(item);
			}
		}

		public static GitIgnoreParser ReadFromFile(string FilePath)
		{
			if (!File.Exists(FilePath))
			{
				throw new FileNotFoundException(@"[{0t not in c:\temp directory]", FilePath);
			}

			string content = "";
			using (StreamReader Reader = new StreamReader(FilePath))
			{
				content = Reader.ReadToEnd();
			}

			GitIgnoreParser parser = new GitIgnoreParser(content);
			return parser;
		}
	}
}
