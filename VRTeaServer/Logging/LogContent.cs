using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.Logging
{
	/// <summary>
	/// ログ単体
	/// </summary>
	internal struct LogContent
	{
		/// <summary>
		/// ログの内容
		/// </summary>
		public string Content { get; set; }
		/// <summary>
		/// ログが作成された時刻
		/// </summary>
		public DateTime Timestamp { get; set; }
		/// <summary>
		/// 接頭辞
		/// </summary>
		public char Prefix { get; set; }

		/// <summary>
		/// 文字列に変換するやつ
		/// </summary>
		/// <returns>文字列</returns>
		public override string ToString()
		{
			return $"{Prefix}:{Content}";
		}
	}
}
