using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using VRTeaServerLib.Service.NetworkService;

namespace VRTeaServerLib.Service
{
	public class Network : IService
	{
		private readonly SessionManager _sessionManager;
		private readonly List<INetworkService> _serverList = [];
		private readonly IGameServer? _gameServer;

		public Network(SessionManager sessionManager, params INetworkService[] services)
		{
			_sessionManager = sessionManager;
			_serverList.AddRange(services);
			// ゲームサーバだけ抽出
			_gameServer = services.OfType<IGameServer>().FirstOrDefault();
		}

		/// <summary>
		/// ネットワークの処理を開始する
		/// </summary>
		/// <param name="cts">キャンセルするやつ</param>
		/// <returns>非同期タスク</returns>
		public async Task Start(CancellationTokenSource cts)
		{
			// Tcpは各サービス分
			List<Task> StartTcpProc()
			{
				List<Task> acceptProcTasks = [];

				async Task AcceptProcAsync(INetworkService service)
				{
					while (true)
					{
						try
						{
							TcpClient tcpClient = await service.Listener.AcceptTcpClientAsync(cts.Token);

							// キープアライブの設定
							tcpClient.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, true);
							tcpClient.Client.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.TcpKeepAliveTime, 5);
							tcpClient.Client.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.TcpKeepAliveInterval, 1);
							tcpClient.Client.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.TcpKeepAliveRetryCount, 3);

							_sessionManager.BeginSession(tcpClient, cts, service.SessionMode);
						}
						catch (OperationCanceledException)
						{
							break;
						}
					}
				}

				foreach (var service in _serverList)
				{
					acceptProcTasks.Add(AcceptProcAsync(service));
				}

				return acceptProcTasks;
			}

			// UDPの処理は１つだけでいい
			Task startUdpProc = Task.Run(async () =>
			{
				if (_gameServer is null)
				{
					Log.Error($"{nameof(_gameServer)} is null.");
					return;
				}

				if (_gameServer.Listener.LocalEndpoint is not IPEndPoint ipEndPoint)
				{
					Log.Error($"{nameof(_gameServer.Listener.LocalEndpoint)} is not IPEndPoint.");
					return;
				}

				UdpClient client = new UdpClient();
				while (true)
				{
					var result = await client.ReceiveAsync();
					result.
				}
			});

			await Task.WhenAll([startUdpProc, ..StartTcpProc()]);
		}
	}
}
