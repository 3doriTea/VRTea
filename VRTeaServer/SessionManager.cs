using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using VRTeaServer.Logging;

namespace VRTeaServer
{
	/// <summary>
	/// <para>セッションを管理するやつ</para>
	/// <para>ゲーム側と通信側の橋渡し役</para>
	/// </summary>
	internal class SessionManager
	{
		public Action<IPEndPoint, int> OnDisconnected { get; } = delegate { };
		private readonly ConcurrentDictionary<int, Session> _sessions = [];
		private readonly ConcurrentDictionary<IPEndPoint, int> _ipepToSessionId = [];
		private int _sessionIdCounter = 0;

		public SessionManager() { }

		public async Task SendEnqueue(int sessionId, SendData data, CancellationTokenSource cts)
		{
			await _sessions[sessionId].SendQueue.Writer.WriteAsync(data, cts.Token);
		}

		/// <summary>
		/// クライアントへの送信キューからDequeue
		/// </summary>
		/// <param name="sessionId"></param>
		/// <param name="cts"></param>
		/// <returns></returns>
		public async Task<SendData> SendDequeue(int sessionId, CancellationTokenSource cts)
		{
			return await _sessions[sessionId].SendQueue.Reader.ReadAsync(cts.Token);
		}

		/// <summary>
		/// クライアントからの受信キューにEnqueue
		/// </summary>
		/// <param name="sessionId"></param>
		/// <param name="data"></param>
		/// <param name="cts"></param>
		/// <returns></returns>
		public async Task ReceiveEnqueue(int sessionId, ReceiveData data, CancellationTokenSource cts)
		{
			await _sessions[sessionId].ReceiveQueue.Writer.WriteAsync(data, cts.Token);
		}

		/// <summary>
		/// クライアントからの受信キューからDequeue
		/// </summary>
		/// <param name="sessionId"></param>
		/// <param name="cts"></param>
		/// <returns></returns>
		public async Task<ReceiveData> ReceiveDequeue(int sessionId, CancellationTokenSource cts)
		{
			return await _sessions[sessionId].ReceiveQueue.Reader.ReadAsync(cts.Token);
		}

		/// <summary>
		/// セッションを開始する
		/// </summary>
		/// <param name="client"></param>
		/// <param name="cts"></param>
		/// <param name="sessionMode"></param>
		/// <returns></returns>
		public int BeginSession(TcpClient client, CancellationTokenSource cts, SessionMode sessionMode)
		{
			int sessionId = Interlocked.Increment(ref _sessionIdCounter);
			var session = new Session(client, sessionId, sessionMode, cts);

			if (client.Client.RemoteEndPoint is not IPEndPoint ipEndPoint)
			{
				Log.Error($"{client.Client.RemoteEndPoint} is not IPEndPoint");
				return 0;
			}

			_ipepToSessionId.AddOrUpdate(
				ipEndPoint,
				sessionId,
				(ipEndPoint, oldSessionId) =>
				{
					EndSession(oldSessionId);
					Log.WriteLine($"The previous session was closed due to a new connection. old-sessionId:{oldSessionId}");
					return sessionId;
				});

			_sessions.AddOrUpdate(
				sessionId,
				session,
				(int id, Session s) =>
				{
					s.Dispose();
					return session;
				});
			return sessionId;
		}

		/// <summary>
		/// セッションを終わらせる
		/// </summary>
		/// <param name="sessionId">セッションId</param>
		public void EndSession(int sessionId)
		{
			_sessions.Remove(sessionId, out var session);

			if (session is null)
			{
				Log.Error("remove session is null");
				return;
			}


			if (session.Client.Client.RemoteEndPoint is not IPEndPoint ipep)
			{
				Log.Error("not found IPEndPoint");
				return;
			}

			_ipepToSessionId.Remove(ipep, out _);
		}
	}
}
