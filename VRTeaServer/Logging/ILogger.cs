using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.Logging
{
	/// <summary>
	/// ロガーに共通するインタフェース
	/// </summary>
	internal interface ILogger
	{
		/// <summary>
		/// 改行付きログ出力
		/// </summary>
		/// <param name="content">ログの内容</param>
		public void WriteLine(string content);
		/// <summary>
		/// 無しログ出力
		/// </summary>
		/// <param name="content">ログの内容</param>
		public void Write(string content);
		/// <summary>
		/// エラーログ出力
		/// </summary>
		/// <param name="content">ログの内容</param>
		public void Error(string content);
	}
}
