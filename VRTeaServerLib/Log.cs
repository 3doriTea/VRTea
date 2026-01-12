using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServerLib
{
	/// <summary>
	/// ログ出力ができる
	/// </summary>
	public static class Log
	{
		private class NullLogger : ILogger
		{
			public void Error(string content)
			{
				throw new NotImplementedException();
			}

			public void Write(string content)
			{
				throw new NotImplementedException();
			}

			public void WriteLine(string content)
			{
				throw new NotImplementedException();
			}
		}

		private static ILogger _instance = new NullLogger();

		internal static void SetLogger(ILogger logger)
		{
			_instance = logger;
		}

		/// <summary>
		/// エラーログを出力
		/// </summary>
		/// <param name="content">内容</param>
		public static void Error(string content) => _instance.Error(content);
		/// <summary>
		/// ログを追記
		/// </summary>
		/// <param name="content">内容</param>
		public static void Write(string content) => _instance.Write(content);
		/// <summary>
		/// ログを出力
		/// </summary>
		/// <param name="content">内容</param>
		public static void WriteLine(string content) => _instance.WriteLine(content);
	}
}
