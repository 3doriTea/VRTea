using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServerLib.CLI
{
	/// <summary>
	/// IPアドレスを尋ねる
	/// </summary>
	public class AskIPAddress : InputProvider
	{
		public IPAddress ipAddress { get; set; } = IPAddress.Any;

		public override void Ask()
		{
			bool isIPAddressSet = false;
			while (isIPAddressSet)
			{
				Console.Write("IPAddress:");
				string? input;
				input = Console.ReadLine();

				if (string.IsNullOrEmpty(input))
				{
					if (AskYesNo("No IP specified. Use IPAddress.Any to allow all access? (Y/n)"))
					{
						ipAddress = IPAddress.Any;
						isIPAddressSet = true;
					}
					else
					{
						// 空の入力だけどAnyを使うわけではない == 間違えてエンター押したんだなーもう一回！
						continue;
					}
				}
				else
				{
					if (IPAddress.TryParse(input, out var inputIPAddress))
					{
						ipAddress = inputIPAddress;
						isIPAddressSet = true;
					}
					else
					{
						Console.WriteLine("Invalid IPAddress format! pls try again.");
						continue;
					}
				}
			}
		}
	}
}
