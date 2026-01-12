using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using VRTeaServerLib.Service.NetworkService;

namespace VRTeaServerLib.Service
{
	public class Network : IService
	{
		private SessionManager _sessionManager;
		private List<INetworkService> _serverList = [];

		public Network(SessionManager sessionManager, params INetworkService[] service)
		{
			_sessionManager = sessionManager;
			_serverList.AddRange(service);
		}

		public async Task Start(CancellationTokenSource cts)
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

			await Task.WhenAll(acceptProcTasks);
		}
	}
}
