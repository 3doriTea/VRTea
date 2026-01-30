using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.Service
{
	/// <summary>
	/// 終了を検知するサービス
	/// </summary>
	internal class TerminateService : IService
	{
		public TerminateService(CancellationTokenSource cts)
		{
			Console.CancelKeyPress += (s, e) =>
			{
				e.Cancel = true;
				cts.Cancel();
			};

			AppDomain.CurrentDomain.ProcessExit += (s, e) =>
			{
				cts.Cancel();
			};
		}

		public async Task Start(CancellationTokenSource cts) => await Task.Run(() =>
		{
			Console.WriteLine("Press [Enter] or [Ctrl+C] to exit.");

			_ = Console.ReadLine();  // ここでエンター送られるまで待つ

			cts.Cancel();
		});
	}
}
