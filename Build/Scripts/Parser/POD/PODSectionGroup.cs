using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace UTVehicles.Automation.Parser.POD
{
	public class PODSectionGroup : PODSectionBase
	{
		private SectionTag TagBegin;
		private SectionTag TagEnd;

		public bool IsVerbatim;
		public bool IsParagraph;

		public PODSectionGroup() { }
		public PODSectionGroup(string[] lines, SectionTag TagS, SectionTag TagE)
		{
			this.Content = lines;
			this.TagBegin = TagS;
			this.TagEnd = TagE;
		}

		public PODSectionGroup(SectionTag TagS, SectionTag TagE)
			: base()
		{
			this.TagBegin = TagS;
			this.TagEnd = TagE;

			IsParagraph = TagS.IsParagraph;
		}

		public override PODSectionGroup Parse<PODSectionGroup>(PODSectionBase Root, PODSectionBase Parent, int FromIndex, int ToIndex)
		{
			if (TagBegin.Type != "")
			{
				return base.Parse<PODSectionGroup>(Root, Parent, FromIndex + 1, ToIndex);
			}

			return base.Parse<PODSectionGroup>(Root, Parent, FromIndex, ToIndex);
		}

		protected override void ParseLines()
		{
			// skip parsing for HTML group
			if (TagBegin.Type == "html")
				return;

			base.ParseLines();
		}

		public override bool ShouldParent(PODSectionBase Previous)
		{
			if (Previous != null && Previous.GetType() == typeof(PODSectionItem))
			{
				var item = (PODSectionItem)Previous;
				if (item != null)
				{
					if (Previous.Sections.Count == 0)
					{
						if (item.Tag.Tag == "item" && IsParagraph)
						{
							Previous.AddChild(this);
							return true;
						}
					}
					else if (ShouldParent(Previous.Sections.Last()))
					{
						return true;
					}
				}
			}

			return false;
		}

		public override string ToFlatten()
		{
			// skip parsing for HTML group
			if (TagBegin.Type == "html")
				return string.Join(Environment.NewLine, GetLines());

			string basestring = base.ToFlatten();
			
			string formatstring = "{0}";
			if (TagBegin != null)
			{
				if (IsVerbatim)
				{
					formatstring = "<pre>{0}</pre>";
				} 
				else if (TagBegin.IsParagraph)
				{
					formatstring = "<p>{0}</p>";
				}
			}

			return string.Format(formatstring, basestring);
		}

		public override string ToText()
		{
			string basestring = base.ToText();
			if (TagBegin.IsParagraph)
			{
				string s = string.Format("{0}{1}", base.ToText(), "\r\n");
				return s;
			}

			return basestring;
		}

		public override string ToString()
		{
			if (TagBegin.IsParagraph)
			{
				string content = base.ToFlatten().TrimStart(" ".ToCharArray());
				int ind = content.IndexOf(" ");
				if (ind > 0 && content.Length > 4*ind)
					content = string.Format("    {0}...", content.Substring(0, ind));
				else if (content.Length > 0)
					content = string.Format("    {0}", content);
				else
					content = "";
				
				return string.Format("{0}{1}", IsVerbatim ? "PRE" : "P", content);
			}

			return string.Format("{0} (Sections: {1})", TagBegin.Type, Sections.Count);
		}
	}
}
