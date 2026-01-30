using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.Service
{
	/// <summary>
	/// TCPを使うゲームサービス
	/// </summary>
	internal class GameTcpService : IService
	{
		public GameTcpService() { }

		public async Task Start(CancellationTokenSource cts) => await Task.Run(() =>
		{

		});
	}
}
