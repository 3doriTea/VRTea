using VRTeaServer.Logging;
using VRTeaServer.Service;
using VRTeaServer.Utility;

namespace VRTeaServer
{
	/// <summary>
	/// ログサービス
	/// </summary>
	internal class LoggerService : IService, ILogger
	{
		public float SaveIntervalSec { get; set; } = 60.0f;
		public int SaveLimitCount { get; set; } = 1000;
		private readonly DirectoryInfo _logDir;
		private readonly List<LogContent> _logList = [];

		public LoggerService(DirectoryInfo logDir)
		{
			_logDir = logDir;
			Log.SetLogger(this);
		}

		/// <summary>
		/// 定期的なログ処理を開始
		/// </summary>
		/// <param name="cts"></param>
		/// <returns></returns>
		public async Task Start(CancellationTokenSource cts)
		{
			Log.WriteLine($"ロガー起動した");
			while (true)
			{
				try
				{
						await Task.Delay(TimeUtil.ToMilliSec(SaveIntervalSec), cts.Token);
						await WriteOutLogs();
				}
				catch (TaskCanceledException)
				{
					Log.WriteLine($"ロガーキャンセルを受信した");
					break;
				}
			}
			Log.WriteLine($"ロガー停止した");
		}

		/// <summary>
		/// 主記憶内のログを補助記憶に書き出す
		/// </summary>
		/// <returns></returns>
		public async Task WriteOutLogs()
		{
			string? logsText;
			int count = 0;
			lock (_logList)
			{
				logsText = string.Join("\n", _logList);
				count = _logList.Count;
				_logList.Clear();
			}

			try
			{
				string fileName = $"{DateTime.Now:yyyy-MM-dd_HH-mm-ss}_{count:00000}.log";
				string filePath = Path.Combine(_logDir.FullName, fileName);
				await File.WriteAllTextAsync(filePath, logsText);
			}
			catch (Exception ex)
			{
				Console.WriteLine(ex);
			}
		}

		/// <summary>
		/// 主記憶に溜めるログ数上限を確認、必要に応じて補助記憶に書き出す
		/// </summary>
		/// <returns></returns>
		private async Task CheckLimit()
		{
			bool needWriteOut = false;
			lock (_logList)
			{
				needWriteOut = _logList.Count >= SaveLimitCount;
			}
			if (needWriteOut)
			{
				await WriteOutLogs();
			}
		}

		/// <summary>
		/// エラーログを出力
		/// </summary>
		/// <param name="content">ログの内容</param>
		public void Error(string content)
		{
			var log = new LogContent
			{
				Prefix = 'E',
				Content = content,
				Timestamp = DateTime.Now
			};
			lock (_logList)
			{
				_logList.Add(log);
			}
			Console.WriteLine(log);
			_ = CheckLimit();
		}

		/// <summary>
		/// 改行無しでログを出力
		/// </summary>
		/// <param name="content">ログの内容</param>
		public void Write(string content)
		{
			bool needNewLine = false;
			lock (_logList)
			{
				needNewLine = _logList.Count == 0 || _logList.Last().Content[^content.Length..] != content;
			}
			if (needNewLine)
			{
				WriteLine(content);
				return;
			}

			lock (_logList)
			{
				var log = new LogContent
				{
					Prefix = '>',
					Content = _logList.Last().Content + content,
					Timestamp = DateTime.Now
				};
				_logList[^1] = log;
			}
			Console.Write(content);
		}

		/// <summary>
		/// 改行付きでログを出力
		/// </summary>
		/// <param name="content">ログの内容</param>
		public void WriteLine(string content)
		{
			var log = new LogContent
			{
				Prefix = '>',
				Content = content,
				Timestamp = DateTime.Now
			};
			lock (_logList)
			{
				_logList.Add(log);
			}
			Console.WriteLine(log);
			_ = CheckLimit();
		}
	}
}
