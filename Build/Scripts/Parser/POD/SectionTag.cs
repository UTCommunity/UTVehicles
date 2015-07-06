using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace UTVehicles.Automation.Parser.POD
{
	public class SectionTag
	{
		public string Tag = string.Empty;
		public string Type = string.Empty;
		public string Text = string.Empty;

		public bool IsGroup = false;
		public bool IsParagraph = false;

		private string[] TagSplit;
		public int LineNumber;

		public SectionTag(string line, int linenum)
		{
			string s = line.TrimStart('=');
			string[] splits = s.Split(" ".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);

			LineNumber = linenum;
			TagSplit = splits;

			if (splits.Length > 1)
			{
				Type = splits[1].ToLower();
			}
			if (splits.Length > 0)
			{
				Tag = splits[0].ToLower();
				Text = string.Join(" ", splits.Skip(1));
				switch (Tag)
				{
					case "begin":
					case "end":
						IsGroup = true;
						break;
				}
			}
			else
			{
				IsParagraph = line.Length == 0;
				IsGroup = IsParagraph;
			}
		}
		public bool IsEndingFor(SectionTag t)
		{
			return IsGroup && t.IsGroup && Type == t.Type && ((Tag.Equals("end") && t.Tag.Equals("begin")) || (t.Tag == Tag));
		}

		public override bool Equals(object obj)
		{
			// If parameter cannot be cast to Point return false.
			SectionTag t = (SectionTag)obj;
			if ((System.Object)t == null)
			{
				return false;
			}

			return Tag.Equals(t.Tag, StringComparison.InvariantCultureIgnoreCase);
		}

		public bool Equals(SectionTag t)
		{
			return Tag.Equals(t.Tag, StringComparison.InvariantCultureIgnoreCase);
		}

		public override int GetHashCode()
		{
			return base.GetHashCode();
		}

		public static bool operator ==(SectionTag a, SectionTag b)
		{
			// If both are null, or both are same instance, return true.
			if (System.Object.ReferenceEquals(a, b))
			{
				return true;
			}

			// If one is null, but not both, return false.
			if (((object)a == null) || ((object)b == null))
			{
				return false;
			}

			return (a.Tag == b.Tag);
		}

		public static bool operator !=(SectionTag a, SectionTag b)
		{
			return !(a == b);
		}

		public override string ToString()
		{
			if (IsParagraph)
				return "Paragraph";
			if (IsGroup)
				return Type + "#" + LineNumber;

			return Tag + "#" + LineNumber;
		}

		internal static bool IsTag(string line)
		{
			return (line.Trim().StartsWith("=") && line.Length > 1)
				|| (line.Length == 0); // paragraph
		}
	}
}
