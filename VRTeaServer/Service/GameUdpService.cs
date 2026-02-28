using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using VRTeaServer.Logging;

namespace VRTeaServer.Service
{
	internal class GameUdpService : IService
	{
		private SessionManager _sessionManager;
		private IPAddress _serverIPAddress;  // サーバーのIPアドレス
		private ushort _gamePort;            // ゲームサービスを公開するポート番号

		public GameUdpService(SessionManager sessionManager, IPAddress serverIPAddress, ushort gamePort)
		{
			_sessionManager = sessionManager;
			_serverIPAddress = serverIPAddress;
			_gamePort = gamePort;
		}

		public async Task Start(CancellationTokenSource cts) => await Task.Run(async () =>
		{
			var ipEndPoint = new IPEndPoint(_serverIPAddress, _gamePort);
			{  // udp client 使用ブロック
				using var udpClient = new UdpClient(ipEndPoint);

				Log.WriteLine($"UDPサーバー起動した");

				try
				{
					await Task.WhenAll(
						Task.Run(async () =>
						{
							while (true)
							{
								var result = await udpClient.ReceiveAsync(cts.Token);
								if (result.RemoteEndPoint is IPEndPoint remoteIPEndPoint)
								{
									await _sessionManager.ReceiveEnqueue(
										remoteIPEndPoint,
										new ReceiveData(result.Buffer),
										cts);
								}
								else
								{
									Log.Error("result.RemoteEndPointのIPEndPointへのキャスト失敗");
								}
							}
						}, cts.Token),
						Task.Run(async () =>
						{
							Log.WriteLine($"送信UDP待機開始");
							while (true)
							{
								try
								{
									// ひたすら送信しまくる
									SendDataWithIPEP sendDataWithIPEP = await _sessionManager.SendDequeueUDP(cts);
									Log.WriteLine($"send UDP at[{sendDataWithIPEP.To}]:{BitConverter.ToString(sendDataWithIPEP.Buffer)}");
									udpClient.Send(sendDataWithIPEP.Buffer, sendDataWithIPEP.To);
								}
								catch (Exception ex)
								{
									Log.Error($"常に送信するUDPから例外:{ex}");
									break;
								}
							}
							Log.WriteLine($"送信UDP待機終了");
						}, cts.Token));

					// クライアントの受付終わったから切断する
					udpClient.Close();
				}
				catch (OperationCanceledException)
				{
					Log.WriteLine($"UDPサーバーキャンセルを受信した");
				}
			}

			Log.WriteLine($"UDPサーバー停止した");
		});
	}
}
