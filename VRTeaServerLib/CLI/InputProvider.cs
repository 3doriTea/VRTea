using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServerLib.CLI
{
	public abstract class InputProvider
	{
		public InputProvider()
		{

		}

		public abstract void Ask();

		/// <summary>
		/// Y/n の回答を待つ
		/// </summary>
		/// <param name="message">質問</param>
		/// <returns>Y/n : true/false</returns>
		public static bool AskYesNo(string message)
		{
			Console.WriteLine(message);

			string? input = null;
			while (string.IsNullOrEmpty(input))
			{
				input = Console.ReadLine();

				if (input == "Y" || input == "n")
				{
					break;  // 求めていた回答がキター
				}
				else
				{
					input = null;
				}
			}

			return input == "Y";
		}
	}
}
