using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.GameLogic
{
	/// <summary>
	/// プレイヤーの初期名を生成するクラス
	/// </summary>
	internal class NameGenerator
	{
		const int WordPickCount = 8;

		static readonly string[] NameKeyWords =
		[
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"まる",
			"なが",
			"ぷい",
			"にゃ",
			"にゅ",
			"にょ",
			"の",
			"ぬ",
			"ん",
		];

		public static string Generate(int id)
		{
			byte[] bytes = Encoding.UTF8.GetBytes($"{id}{id}{id}{int.MaxValue - id}{id}{id}{int.MaxValue - id}");
			byte[] hashBytes = SHA256.HashData(bytes);

			string hash = Convert.ToHexString(hashBytes).ToLower();
			var buffer = new string[WordPickCount];
			for (int i = 0; i < buffer.Length; i++)
			{
				buffer[i] = NameKeyWords[int.Parse($"{hash[i]}", System.Globalization.NumberStyles.HexNumber)];
			}

			return string.Join("", buffer);
		}
	}
}
