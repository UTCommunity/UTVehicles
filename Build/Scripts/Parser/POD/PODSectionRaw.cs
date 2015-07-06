using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace UTVehicles.Automation.Parser.POD
{
	public class PODSectionRaw : PODSectionBase
	{
		public bool IsVerbatim = false;

		protected override void ParseLines()
		{
			// Check if raw text is verbatim formatted
			if (CheckIfVerbatim() && Parent.GetType() == typeof(PODSectionGroup))
			{
				((PODSectionGroup)Parent).IsVerbatim = true;
			}
		}

		private bool CheckIfVerbatim()
		{
			string templine = GetLine(ContentFromIndex);
			string trimmed = templine.TrimStart(" \t".ToCharArray());
			string prefix = templine.Substring(0, templine.Length - trimmed.Length);

			if (prefix.Length > 0)
			{
				for (int i = ContentFromIndex; i < ContentToIndex; i++)
				{
					string line = GetLine(i);
					if (!line.StartsWith(prefix))
					{
						return false;
					}
				}

				return true;
			}

			return false;
		}

		public override string ToFlatten()
		{
			return string.Join(Environment.NewLine, GetLines().Select(l => (IsVerbatim ? l : ConvertFormattingCodes(l))));
		}

		public override string ToText()
		{
			return string.Join("\r\n", GetLines().Select(l => (IsVerbatim ? l : ConvertFormattingCodes(l, true))));
		}

		public override string ToString()
		{
			return string.Format("Raw section (Lines: {0})", ContentToIndex - ContentFromIndex);
		}
	}
}
