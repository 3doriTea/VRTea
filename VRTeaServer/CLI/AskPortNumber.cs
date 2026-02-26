using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.CLI
{
	/// <summary>
	/// ポート番号を質問する
	/// </summary>
	internal class AskPortNumber : AskBase
	{
		public ushort PortNumber { get; private set; }

		public override void Ask()
		{
			bool isPortNumberSet = false;
			while (!isPortNumberSet)
			{
				Console.Write("Port:");
				string? input = null;
				input = Console.ReadLine();

				if (string.IsNullOrEmpty(input))
				{
					continue;  // 空文字だったらもう一度尋ねる
				}

				if (ushort.TryParse(input, out var inputPortNumber))
				{
					PortNumber = inputPortNumber;
					isPortNumberSet = true;
				}
				else
				{
					Console.WriteLine("Invalid Port Number!! pls try again.");
					continue;
				}
			}
		}
	}
}
