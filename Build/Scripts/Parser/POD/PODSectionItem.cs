using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace UTVehicles.Automation.Parser.POD
{
	public class TagRender
	{
		protected string Tag;
		public string Extra;

		public TagRender(string Tag)
		{
			SetTag(Tag);
		}

		public virtual void SetTag(string Tag)
		{
			this.Tag = Tag;
		}

		public virtual string GetCurrentType()
		{
			return Tag;
		}

		public virtual string ToFlatten()
		{
			return "";
		}

		public virtual string ToFlatten(List<PODSectionBase> Sections)
		{
			return ToFlatten();
		}

		public override string ToString()
		{
			return Tag; // string.Format("", Text);
		}

		public virtual string ToText()
		{
			return "";
		}

		public virtual string ToText(List<PODSectionBase> Sections)
		{
			return ToText();
		}
	}

#region " Tag Render Elements "

	public class TagRenderHead : TagRender
	{
		public TagRenderHead(int level, string text) : base("head" + level)
		{
			this.Level = level;
			this.Text = text;
		}

		public int Level { get; set; }
		public string Text { get; set; }

		public override string ToFlatten()
		{
			return string.Format("<h{0}>{1}</h{0}>", Level, Text);
		}

		public override string ToString()
		{
			return string.Format("H{0}    Title: {1}", Level, Text);
		}

		public override string ToText()
		{
			string pref = "";
			string suff = "";
			switch (Level)
			{
				case 1:
					pref = "\r\n\r\n";
					suff = "\r\n";
					break;
				case 2:
					pref = "\r\n\r\n";
					suff = "\r\n";
					break;
				case 3:
					pref = "\r\n";
					suff = "\r\n";
					break;
				default:
					pref = "\r\n";
					suff = "";
					break;
			}

			string s = pref + PODSectionBase.ConvertFormattingCodes(Text, true) + suff;
			return s;
		}
	}

	public abstract class TagRenderListBase : TagRender
	{
		public string Type { get; set; }
		public string ItemType { get; set; }
		public bool MergeSub { get; set; }

		public TagRenderListBase(string Tag)
			: base(Tag)
		{
			Type = Tag;
		}

		public override void SetTag(string Tag)
		{
			base.SetTag(Tag);
			this.Type = Tag;

			switch (Tag.ToLower())
			{
				case "li":
					MergeSub = true;
					break;
				default:
					MergeSub = false;
					break;
			}
		}

		public static string GetListType(string Type)
		{
			if (Type == "1")
				return "ol";
			else if (Type == "*")
				return "ul";

			return "dl";
		}

		public static string GetItemType(string Type)
		{
			if (Type == "1" || Type == "*")
				return "li";

			return "dt";
		}

		public static string GetSubType(string Type)
		{
			return "dd";
		}
	}
	public class TagRenderList : TagRenderListBase
	{
		public TagRenderList(bool StartTag)
			: base(GetListType(""))
		{
			this.StartTag = StartTag;
			this.Indent = "4";
		}

		public bool StartTag { get; set; }
		public string ItemTag { get; set; }
		public string Indent { get; set; }

		//public static string GetItemType()
		//{
		//	return "dd";
		//}

		public TagRenderList SetIndent(string IndentLevel)
		{
			this.Indent = IndentLevel;
			return this;
		}

		public override string ToFlatten()
		{
			string tag = StartTag ? "<{0}>" : "</{0}>";
			return string.Format(tag, Tag);
		}

		public override string ToFlatten(List<PODSectionBase> Sections)
		{
			string s = ToFlatten();

			if (ItemTag != null)
			{
				s += string.Concat(Sections.Select(o => string.Format("<{0}>{1}</{0}>", ItemTag, o.ToFlatten())));
			}
			else
			{
				s += string.Concat(Sections.Select(o => o.ToFlatten()));
			}
			
			return s;
		}

		public override string ToText()
		{
			return "";
		}

		public override string ToText(List<PODSectionBase> Sections)
		{
			string s = ToText();

			if (ItemTag != null)
			{
				s += string.Concat(Sections.Select(o => string.Format("- {0}", ItemTag, o.ToText())));
			}
			else
			{
				s += string.Concat(Sections.Select(o => o.ToText()));
			}

			return s;
		}

		public override string ToString()
		{
			return string.Format("{0}    {1}", Type, ToFlatten());
		}
	}

	public class TagRenderListItem : TagRenderListBase
	{
		public TagRenderListItem(string text)
			: base(GetItemType(text))
		{
			this.Text = text;
			this.Extra = text;
		}

		public string Text { get; set; }

		public override string GetCurrentType()
		{
			return Tag;
		}

		public override string ToFlatten()
		{
			return string.Format("<{0}>{1}</{0}>", Tag, PODSectionBase.ConvertFormattingCodes(Text));
		}

		public override string ToFlatten(List<PODSectionBase> Sections)
		{
			if (MergeSub)
			{
				string inner = Sections.Count == 1 ? Sections[0].ToText() : string.Concat(Sections.Select(o => o.ToFlatten()));
				return string.Format("<{0}>{1}</{0}>", Type, inner);
			}

			string s = ToFlatten();
			s += string.Concat(Sections.Select(o => string.Format("<{0}>{1}</{0}>", GetSubType(Tag), o.ToFlatten())));
			return s;
		}

		public override string ToText()
		{
			return "- " + PODSectionBase.ConvertFormattingCodes(Text, true);
		}

		public override string ToText(List<PODSectionBase> Sections)
		{
			if (MergeSub)
			{
				string inner = Sections.Count == 1 ? Sections[0].ToText() : string.Concat(Sections.Select(o => o.ToText()));
				return string.Format("- {0}", inner);
			}

			return ToText() + "\r\n" + string.Concat(Sections.Select(o => string.Format("  {0}" + "\r\n", o.ToText().TrimEnd("\r\n".ToCharArray()).Replace("\r\n", "\r\n  "))));
		}

		public override string ToString()
		{
			return string.Format("{0}    Text: {1}", Tag, Text);
		}
	}

#endregion 

	public class PODSectionItem : PODSectionBase
	{
		public SectionTag Tag;
		public TagRender Render;

		public PODSectionItem(PODSectionBase Root, PODSectionBase Parent, SectionTag Tag)
		{
			this.Root = Root;
			this.Parent = Parent;

			this.Tag = Tag;

			switch (Tag.Tag)
			{
				case "head1": Render = new TagRenderHead(1, Tag.Text); break;
				case "head2": Render = new TagRenderHead(2, Tag.Text); break;
				case "head3": Render = new TagRenderHead(3, Tag.Text); break;
				case "head4": Render = new TagRenderHead(4, Tag.Text); break;

				case "over": Render = new TagRenderList(true).SetIndent(Tag.Type);break;
				case "item": Render = new TagRenderListItem(Tag.Text); break;
				case "back": Render = new TagRenderList(false).SetIndent(Tag.Type); break;

				case "for": Render = new TagRender(Tag.Tag); break;

				case "pod": Render = new TagRender(Tag.Tag); break;
				case "cut": Render = new TagRender(Tag.Tag); break;

				case "encoding": ; break;
			}
		}

		public override string[] GetLines()
		{
			return new string[] { };
		}

		public string GetListType()
		{
			if (Render != null)
			{
				if (Render.GetType() == typeof(TagRenderList))
				{
					return ((TagRenderList)Render).Type;
				}
				else if (typeof(TagRenderListBase).IsAssignableFrom(Render.GetType()))
				{
					return TagRenderListBase.GetListType(Render.Extra);
				}
				else
				{
					return Render.GetCurrentType();
				}
			}
			
			return Tag.Type;
		}

		public override string ToFlatten()
		{
			return Render != null ? Render.ToFlatten(Sections) : Tag.Text;
		}

		public override string ToString()
		{
			return Render != null ? Render.ToString() : string.Format("Item {0} - {1})", "", Tag != null ? Tag.ToString() : "");
		}

		public override string ToText()
		{
			if (Tag.Tag == "encoding")
				return "";

			return Render != null ? Render.ToText(Sections) : ConvertFormattingCodes(Tag.Text, true);
		}

		public override bool ShouldParent(PODSectionBase Previous)
		{
			if (Previous != null && Previous.GetType() == typeof(PODSectionItem))
			{
				//if (Previous.Sections.Count == 0)
				{
					var item = (PODSectionItem)Previous;
					if (item != null && Tag.Tag == "item" && item.Tag.Tag == "over")
					{
						Previous.AddChild(this);
						return true;
					}
					else if (item != null && Tag.Tag == "back" && item.Tag.Tag == "over")
					{
						var thistype = GetListType();
						var othertype = item.GetListType();
						if (thistype != othertype)
						{
							OverrideType(othertype);
						}
					}
				}
				//else
				//{
				//	return ShouldParent(Previous.Sections.Last());
				//}
			}

			return false;
		}

		public override void AddChild(PODSectionBase NewChild)
		{
			base.AddChild(NewChild);

			// check if list tag is given, and no sub-sections already added
			if (Tag.Tag == "over" && Sections.Count == 1 && typeof(PODSectionItem).IsAssignableFrom(NewChild.GetType()))
			{
				var item = (PODSectionItem)NewChild;
				if (item != null)
				{
					var thistype = GetListType();
					var othertype = item.GetListType();
					if (thistype != othertype)
					{
						OverrideType(othertype);
					}
				}
			}
		}

		private void OverrideType(string Type)
		{
			if (Render != null)
			{
				Render.SetTag(Type);
			}
		}
	}
}
