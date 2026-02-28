using System;
using System.Buffers.Binary;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using VRTeaServer.Exceptions;
using VRTeaServer.Logging;
using static VRTeaServer.Utility.NetWorkUtil;

namespace VRTeaServer.Service
{
	/// <summary>
	/// TCPを使うゲームサービス
	/// </summary>
	internal class GameTcpService : IService
	{
		private readonly SessionManager _sessionManager;
		private IPAddress _serverIPAddress;  // サーバーのIPアドレス
		private ushort _gamePort;            // ゲームサービスを公開するポート番号
		const int BufferSize = 1024;         // 溜めておくバッファのサイズ

		public GameTcpService(SessionManager sessionManager, IPAddress serverIPAddress, ushort gamePort)
		{
			_sessionManager = sessionManager;
			_serverIPAddress = serverIPAddress;
			_gamePort = gamePort;
		}

		public async Task Start(CancellationTokenSource cts) => await Task.Run(async () =>
		{
			using var listener = new TcpListener(_serverIPAddress, _gamePort);

			listener.Start();

			Log.WriteLine($"TCPサーバー起動した");
			while (true)
			{
				try
				{
					TcpClient client = await listener.AcceptTcpClientAsync(cts.Token);
					// クライアント受け付けたら即セッション開始
					_ = StartSession(client, cts);
				}
				catch (OperationCanceledException)
				{
					Log.WriteLine($"TCPサーバーキャンセルを受信した");
					break;  // キャンセル発動されたらサービス止める
				}
			}
			Log.WriteLine($"TCPサーバー停止した");
			listener.Stop();
			listener.Dispose();
		});

		/// <summary>
		/// クライアントとセッション開始
		/// </summary>
		/// <param name="client"></param>
		/// <param name="cts"></param>
		/// <returns></returns>
		private async Task StartSession(TcpClient client, CancellationTokenSource cts)
		{
			int sessionId = _sessionManager.BeginSession(client, cts, SessionMode.Game);
			Log.WriteLine($"クライアント受け付けた{client.Client.RemoteEndPoint}");

			try
			{
				NetworkStream stream = client.GetStream();
				byte[] buffer = new byte[BufferSize];

				try
				{
					await Task.WhenAll(
						Task.Run(async () =>
						{
							while (true)
							{
								try
								{
									byte[] totalSizeBuffer = new byte[sizeof(int)];
									if (!await ReadExactlyAsync(stream, totalSizeBuffer, cts))
									{
										break;  // 切断されたら終了
									}

									int totalSize = BinaryPrimitives.ReadInt32BigEndian(totalSizeBuffer);
									int bodySize = totalSize - sizeof(int);

									if (bodySize <= 0)
									{
										continue;  // ボディサイズがおかしいなら無視
									}

									byte[] bodyBuffer = new byte[bodySize];
									if (!await ReadExactlyAsync(stream, bodyBuffer, cts))
									{
										break;  // 切断されたら終了
									}

									// 2つを合わせたバッファを作成
									byte[] combined = new byte[totalSize];
									totalSizeBuffer.AsSpan().CopyTo(combined.AsSpan().Slice(0, totalSizeBuffer.Length));
									bodyBuffer.AsSpan().CopyTo(combined.AsSpan().Slice(totalSizeBuffer.Length));


									await _sessionManager.ReceiveEnqueue(sessionId, new ReceiveData(combined), cts);
								}
								catch (IOException)
								{
									break;
								}
								catch (Exception ex)
								{
									Console.WriteLine(ex.ToString());
								}
							}
						}, cts.Token),
						Task.Run(async () =>
						{
							//try
							//{
							while (true)
							{
								try
								{
									SendData sendData = await _sessionManager.SendDequeue(sessionId, cts);
									Log.WriteLine($"send TCP at[{sessionId}]:{sendData.GetString()}");
									Log.WriteLine($"send TCP at[{sessionId}]:{BitConverter.ToString(sendData.Buffer)}");

									await stream.WriteAsync(sendData.Buffer, cts.Token);
								}
								catch (IOException)
								{
									break;
								}
								catch (Exception ex)
								{
									Console.WriteLine(ex.ToString());
								}
							}
							//}
							//catch (SessionKeyNotFoundException)
							//{
							//	cts.Cancel();
							//	return;
							//}
						}, cts.Token));

					// クライアントの受付終わったから切断する
					_sessionManager.Disconnect(sessionId);
				}
				catch (OperationCanceledException ex)
				{
					_ = ex;
					return;
				}
				catch (IOException ex)
				{
					Console.WriteLine($"{ex}");
				}
			}
			catch (Exception ex)
			{
				Log.Error($"{ex}");
			}
		}
	}
}
