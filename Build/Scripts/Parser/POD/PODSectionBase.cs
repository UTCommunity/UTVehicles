using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace UTVehicles.Automation.Parser.POD
{
	public class PODSectionBase
	{
		public PODSectionBase Root;
		public PODSectionBase Parent;
		public List<PODSectionBase> Sections = new List<PODSectionBase>();

		public string[] Content = new string[] { };
		public int ContentFromIndex;
		public int ContentToIndex;

		public bool ReferenceContent;

		public string Flat
		{
			get
			{
				return ToFlatten();
			}
		}

		public string Text
		{
			get
			{
				return ToText();
			}
		}

		public string[] Lines
		{
			get
			{
				return GetLines();
			}
		}

		public string[] LinesLast10
		{
			get
			{
				int l = Math.Max(0, Content.Length - 10);
				return GetLines().Skip(l).Take(10).ToArray();
			}
		}

		public string[] LinesFirst10
		{
			get
			{
				return GetLines().Take(10).ToArray();
			}
		}

		public PODSectionBase()
		{
			ReferenceContent = true;
		}

		public PODSectionBase(string[] Content)
		{
			this.Root = this;
			this.Content = Content;
			this.ContentFromIndex = 0;
			this.ContentToIndex = Content.Length;

			ParseLines();
		}

		public string GetLine(int index)
		{
			if (ReferenceContent)
				return Root.GetLine(index);

			return Content[index];
		}

		public virtual string[] GetLines()
		{
			if (ReferenceContent)
			{
				return Root.GetLinesInternal(ContentFromIndex, ContentToIndex);
			}

			return GetLinesInternal(ContentFromIndex, ContentToIndex);
		}

		private string[] GetLinesInternal(int From, int To)
		{
			var arr = Content.Skip(From).Take(To == From ? 1 : To - From).ToArray();
			return arr;
		}

		protected virtual void ParseLines()
		{
			var stack = new List<SectionTag>();

			var MinI = ContentFromIndex;
			var MaxI = ContentToIndex;
			var WasParagraph = false;
			var WasItem = false; // set to check for additional empty line
			var FullText = new List<string>();
			for (int i = MinI; i < MaxI; i++)
			{
				string line = GetLine(i);
				FullText.Add(line);

				// prevent empty paragraph after a paragraph
				if (line == "" && WasParagraph) continue;

				if (!SectionTag.IsTag(line) || (line == "" && WasItem))
				{
					if (WasParagraph || (i == 0 && !ReferenceContent))
					{
						var firstpara = new SectionTag("", i);
						stack.Add(firstpara);
						WasParagraph = false;
						WasItem = false;
					}

					WasParagraph = WasItem;
					WasItem = false;

					continue;
				}

				var tag = new SectionTag(line, i);
				var lasttag = stack.Count > 0 ? stack.Last() : null;
				if (lasttag != null && tag.IsEndingFor(lasttag))
				{
					var NewGroup = new PODSectionGroup(lasttag, tag).
						Parse<PODSectionGroup>(Root, this, lasttag.LineNumber, tag.LineNumber);

					if (!NewGroup.ShouldParent(Sections.Count > 0 ? Sections.Last() : null)) Sections.Add(NewGroup);
					stack.RemoveAt(stack.Count - 1);

					MinI = i+1;

					WasParagraph = tag.IsParagraph;
					WasItem = true;
				}
				else if (!tag.IsGroup)
				{
					WasParagraph = false;
					WasItem = true;

					MinI = i+1;

					var NewItem = new PODSectionItem(Root, this, tag);
					if (!NewItem.ShouldParent(Sections.Count > 0 ? Sections.Last() : null)) Sections.Add(NewItem);
				}
				else
				{
					WasParagraph = false;
					stack.Add(tag);

					if (!tag.IsParagraph)
					{
						// fast forward  to find closing tag
						for (int j = i+1; i < MaxI; j++)
						{
							string endline = GetLine(j);
							if (SectionTag.IsTag(endline))
							{
								var endtag = new SectionTag(endline, j);
								if (endtag.IsEndingFor(tag))
								{
									i = j-1;
									break;
								}
							}
						}
					}
				}
			}

			if (MinI <= MaxI && (Parent != null || MaxI < ContentToIndex))
			{
				Sections.Add(new PODSectionRaw().Parse<PODSectionRaw>(Root, this, MinI, MaxI));
			}
		}
		
		public virtual T Parse<T>(PODSectionBase Root, PODSectionBase Parent, int FromIndex, int ToIndex) where T : PODSectionBase
		{
			this.Root = Root;
			this.Parent = Parent;

			ContentFromIndex = FromIndex;
			ContentToIndex = ToIndex;

			ParseLines();

			return (T)this;
		}

		public virtual bool ShouldParent(PODSectionBase Previous)
		{
			return false;
		}

		public override string ToString()
		{
			return (Parent == null ? "Root " : "")  + string.Format("Sections: {0}", Sections.Count);
		}

		public virtual string ToFlatten()
		{
			return string.Join(Environment.NewLine, Sections.Select(s => s.ToFlatten()));
		}

		public virtual string ToText()
		{
			return string.Join("\r\n", Sections.Select(s => s.ToText()));
		}

		public static Dictionary<string, string> FORMATTING_CODES_MAPPING_HTML = new Dictionary<string, string>()
		{
			// links
			{ @"L<([^ ]*)>", "<a href=\"{0}\">{0}</a>" },

			// code
			{ @"C<([A-Za-z_0-9]*)>", "<code>{0}</code>" },

			// bold, italic, 
			{ @"B<(.*)>", "<strong>{0}</strong>" },
			{ @"I<(.*)>", "<i>{0}</i>" },
		};

		public static Dictionary<string, string> FORMATTING_CODES_MAPPING_TEXT = new Dictionary<string, string>()
		{
			// links
			{ @"L<([^ ]*)>", "{0}" },

			// code
			{ @"C<([A-Za-z_0-9]*)>", "{0}" },

			// bold, italic, 
			{ @"B<(.*)>", "{0}" },
			{ @"I<(.*)>", "{0}" },
		};

		public static string ConvertFormattingCodes(string InText, bool OnlyText = false)
		{
			var mapping = OnlyText ? FORMATTING_CODES_MAPPING_TEXT : FORMATTING_CODES_MAPPING_HTML;
			foreach (var entry in mapping)
			{
				InText = Regex.Replace(InText, entry.Key, delegate(Match match)
				{
					string r = match.ToString();
					if (match.Groups.Count > 1)
					{
						r = string.Format(entry.Value, match.Groups[1].Value.ToString());
					}

					return r;
				}); 
			}

			return InText;
		}

		public virtual void AddChild(PODSectionBase NewChild)
		{
			Sections.Add(NewChild);
			NewChild.SetParent(this);
		}

		protected virtual void SetParent(PODSectionBase NewParent)
		{
			Parent = NewParent;
		}
	}
}
