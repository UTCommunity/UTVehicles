using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace UTVehicles.Automation.Parser
{
	public abstract class BaseParser
	{
		public BaseParser()
		{
		}

		public BaseParser(TextReader reader)
		{
			Parse(reader.ToString());
		}

		public static T Create<T>(TextReader reader) where T : BaseParser, new()
		{
			var obj = new T();
			obj.Parse(reader);
			return obj;
		}

		public BaseParser(string content)
			: this()
		{
			Parse(content);
		}

		public BaseParser(string[] lines)
			: this()
		{
			Parse(lines);
		}

		protected void Parse(TextReader reader)
		{
			var lines = new List<string>();

			while (reader.Peek() > 0)
			{
				lines.Add(reader.ReadLine());
			}

			reader.Dispose();
			PreParse(ref lines);
			Parse(lines.ToArray());
		}

		protected virtual void PreParse(ref List<string> Lines)
		{

		}

		protected void Parse(string content)
		{
			string[] lines = content.Split(Environment.NewLine.ToCharArray());
			Parse(lines);
		}

		protected abstract void Parse(string[] lines);

	}
}
