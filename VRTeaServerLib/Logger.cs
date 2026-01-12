using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServerLib
{
	/// <summary>
	/// <para>ログ使うやつ</para>
	/// <para>自動セーブを行う(主記憶から補助記憶へ書き出す)</para>
	/// </summary>
	public class Logger : IService, ILogger
	{
		/// <summary>
		/// ログ単体
		/// </summary>
		public struct Log
		{
			public string Content { get; set; }
			public DateTime Timestamp { get; set; }
			public char Prefix { get; set; }
		}

		public float SaveIntervalSec { get; set; }  // 自動セーブする時間間隔(秒)
		public int SaveLimitCount { get; set; }     // 自動セーブするログの数
		private readonly DirectoryInfo _logDir;
		private readonly List<Log> _logList = [];

		public Logger(DirectoryInfo logDir, float saveIntervalSec, int saveLimitCount)
		{
			_logDir = logDir;
			SaveIntervalSec = saveIntervalSec;
			SaveLimitCount = saveLimitCount;
		}

		/// <summary>
		/// 定期的なログ処理を開始
		/// </summary>
		/// <param name="cts"></param>
		/// <returns></returns>
		public async Task Start(CancellationTokenSource cts)
		{
			try
			{
				while (true)
				{
					await Task.Delay(TimeUtil.ToMilliSec(SaveIntervalSec), cts.Token);
					await WriteOutLogs();
				}
			}
			catch (TaskCanceledException)
			{
				return;
			}
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

		public void WriteLine(string content)
		{
			var log = new Log
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
				Log log = new Log
				{
					Prefix = '>',
					Content = _logList.Last().Content + content,
					Timestamp = DateTime.Now
				};
				_logList[^1] = log;
			}
			Console.Write(content);
		}

		public void Error(string content)
		{
			Log log = new Log
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
	}
}
