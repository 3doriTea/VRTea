using System;

namespace VRTeaServer.CLI
{
	/// <summary>
	/// CLIの質問ベースクラス
	/// </summary>
	internal abstract class AskBase
	{
		/// <summary>
		/// Y/n の質問をする
		/// </summary>
		/// <param name="message"></param>
		/// <returns>Y / n = true / false</returns>
		public static bool AskYesNo(string message)
		{
			string? input = null;

			while (input is null)
			{
				Console.WriteLine(message);
				input = Console.ReadLine();

				if (input == "Y")
				{
					return true;
				}
				else if (input == "n")
				{
					break;
				}

				input = null;
			}

			return false;
		}

		/// <summary>
		/// 質問
		/// </summary>
		public abstract void Ask();
	}
}
