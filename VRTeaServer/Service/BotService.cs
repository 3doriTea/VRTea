using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.Service
{
	/// <summary>
	/// ボットのサービス
	/// </summary>
	internal class BotService : IService
	{
		public async Task Start(CancellationTokenSource cts) => await Task.Run(() =>
		{

		});
	}
}
