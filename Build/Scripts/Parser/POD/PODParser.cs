using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace UTVehicles.Automation.Parser.POD
{
	public class PODParser : BaseParser
	{
		protected List<PODSectionBase> Sections = new List<PODSectionBase>();

		public PODSectionBase Root;

		protected override void PreParse(ref List<string> Lines)
		{
			// add additional line for group parsing
			if (Lines != null && Lines.Last() != "")
			{
				Lines.Add("");
			}
		}

		protected override void Parse(string[] lines)
		{
			Root = new PODSectionBase(lines);
		}

		public string Flatten()
		{
			return Root.ToFlatten();
		}
		public string Text()
		{
			return Root.ToText();
		}

		//public string Text
		//{
		//	get
		//	{
		//		return Root.ToText();
		//	}
		//}
	}
}
