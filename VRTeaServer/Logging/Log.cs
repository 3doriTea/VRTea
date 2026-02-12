using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.Logging
{
	/// <summary>
	/// ログを扱う
	/// </summary>
	internal static class Log
	{
		private static ILogger _logger = new NullLogger();

		/// <summary>
		/// <para>本来隠れているべきメソッドのため、使わないでください</para>
		/// <para>internal指定です</para>
		/// </summary>
		internal static void SetLogger(ILogger logger)
		{
			_logger = logger;
		}

		/// <summary>
		/// 改行付きのログを出力する
		/// </summary>
		/// <param name="content">ログの内容</param>
		public static void WriteLine(string content) => _logger.WriteLine(content);
		/// <summary>
		/// 改行無しのログを出力する
		/// </summary>
		/// <param name="content">ログの内容</param>
		public static void Write(string content) => _logger.Write(content);
		/// <summary>
		/// エラーログを出力する
		/// </summary>
		/// <param name="content">ログの内容</param>
		public static void Error(string content) => _logger.Error(content);
	}
}
