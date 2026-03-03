using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.CLI
{
	internal class AskAPIKey : AskBase
	{
		public string Key { get; private set; } = string.Empty;
		public override void Ask()
		{
			Console.WriteLine("Groq API Key (Press Enter to skip):");
			var input = Console.ReadLine();

			if (string.IsNullOrEmpty(input))
			{
				return;
			}

			Key = input;
		}
	}
}
