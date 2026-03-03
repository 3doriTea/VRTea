using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Net.Sockets;
using System.Text;
using System.Threading.Channels;
using System.Threading.Tasks;
using VRTeaServer.Exceptions;
using VRTeaServer.Logging;

namespace VRTeaServer
{
	/// <summary>
	/// <para>セッションを管理するやつ</para>
	/// <para>ゲーム側と通信側の橋渡し役</para>
	/// </summary>
	internal class SessionManager
	{
		public class SessionsEnum(ConcurrentDictionary<int, Session> sessions) : IEnumerable<int>
		{
			private ConcurrentDictionary<int, Session> _sessions = sessions;

			public IEnumerator<int> GetEnumerator() => _sessions.Keys.GetEnumerator();
			
			// NOTE: 旧来のコレクション互換性のためのやつ
			IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

			public int Count { get => _sessions.Count; }
		}

		public SessionsEnum Sessions { get; private set; }
		public Action<IPEndPoint, int> OnDisconnected { get; set; } = delegate { };
		private readonly ConcurrentDictionary<int, Session> _sessions = [];
		private readonly ConcurrentDictionary<IPEndPoint, int> _ipepToSessionId = [];
		/// <summary>
		/// UDP用の送信キュー
		/// </summary>
		private Channel<SendDataWithIPEP> _sendQueueUDP = Channel.CreateUnbounded<SendDataWithIPEP>();
		private int _sessionIdCounter = 0;
		private ConcurrentDictionary<int, ReceiveData> _receiveDictUDP = [];

		public SessionManager()
		{
			Sessions = new SessionsEnum(_sessions);
		}
		
		/// <summary>
		/// 堂々と切断する
		/// </summary>
		/// <param name="sessionId">セッションId</param>
		public void Disconnect(int sessionId)
		{
			_sessions.TryGetValue(sessionId, out var session);

			if (session is null)
			{
				return;  // そもそも今セッションがないよ！
			}

			if (session.Client.Client.RemoteEndPoint is IPEndPoint ipEP)
			{
				//　セッションもIPEPも見つけたため切断イベント発動できる
				OnDisconnected.Invoke(ipEP, sessionId);

				// IPEPとセッションの紐づけも削除
				_ipepToSessionId.Remove(ipEP, out _);
			}
			// セッションも削除 ここでTcp通信切断が呼ばれるはず
			_sessions.Remove(sessionId, out _);
		}

		/// <summary>
		/// TCPの送信キューにエンキューする
		/// </summary>
		/// <param name="sessionId"></param>
		/// <param name="data"></param>
		/// <param name="cts"></param>
		/// <returns></returns>
		public async Task SendEnqueue(int sessionId, SendData data, CancellationTokenSource cts)
		{
			await _sessions[sessionId].SendQueue.Writer.WriteAsync(data, cts.Token);
		}

		/// <summary>
		/// UDPの送信キューにエンキューする
		/// </summary>
		/// <param name="sessionId"></param>
		/// <param name="data"></param>
		/// <param name="cts"></param>
		/// <returns></returns>
		public async Task SendEnqueueUDP(int sessionId, SendData data, CancellationTokenSource cts)
		{
			if (_sessions[sessionId].Client.Client.RemoteEndPoint is IPEndPoint remoteIPEP)
			{
				var dataWithIPEP = data.WithIPEP(remoteIPEP);
				// UDP用にIPEPのペアで渡す
				await _sendQueueUDP.Writer.WriteAsync(dataWithIPEP, cts.Token);
			}
			else
			{
				Log.Error("ReceiveEnqueue from IPEP でIPEPと sessionIdの変換に失敗");
			}
		}

		/// <summary>
		/// 試しにセッションを取得する
		/// </summary>
		/// <param name="sessionId">セッションId</param>
		/// <param name="cts"></param>
		/// <param name="session"></param>
		/// <returns>取得に成功した true / false</returns>
		public bool TryGet(int sessionId, CancellationTokenSource cts, out Session? session)
		{
			return _sessions.TryGetValue(sessionId, out session);
		}

		/// <summary>
		/// 試しにセッション情報を更新する
		/// </summary>
		/// <param name="sessionId">セッションId</param>
		/// <param name="session">置換するセッション</param>
		/// <param name="compSession">一致を確認するセッションデータ</param>
		/// <returns>更新に成功した true / false</returns>
		public bool Update(int sessionId, Session session, Session compSession)
		{
			return _sessions.TryUpdate(sessionId, session, compSession);
		}

		/// <summary>
		/// TCP: クライアントへのデータを送信キューからDequeue
		/// </summary>
		/// <param name="sessionId"></param>
		/// <param name="cts"></param>
		/// <returns></returns>
		public async Task<SendData> SendDequeue(int sessionId, CancellationTokenSource cts)
		{
			//if (!_sessions.ContainsKey(sessionId))
			//{
			//	throw new SessionKeyNotFoundException("KeyNotFound");
			//}
			return await _sessions[sessionId].SendQueue.Reader.ReadAsync(cts.Token);
		}

		/// <summary>
		/// UDP: クライアントへのデータを送信キューからD
		/// </summary>
		/// <param name="cts"></param>
		/// <returns></returns>
		public async Task<SendDataWithIPEP> SendDequeueUDP(CancellationTokenSource cts)
		{
			return await _sendQueueUDP.Reader.ReadAsync(cts.Token);
		}

		/// <summary>
		/// クライアントからのデータを受信キューにEnqueue
		/// </summary>
		/// <param name="sessionId"></param>
		/// <param name="data"></param>
		/// <param name="cts"></param>
		/// <returns></returns>
		public async Task ReceiveEnqueue(int sessionId, ReceiveData data, CancellationTokenSource cts)
		{
			_sessions[sessionId].Timestamp = DateTime.Now;
			await _sessions[sessionId].ReceiveQueue.Writer.WriteAsync(data, cts.Token);
		}

		/// <summary>
		/// クライアントからのデータを受信キューにEnqueue
		/// </summary>
		/// <param name="remoteIPEndPoint"></param>
		/// <param name="data"></param>
		/// <param name="cts"></param>
		/// <returns></returns>
		public async Task ReceiveEnqueueUDP(IPEndPoint remoteIPEndPoint, ReceiveData data, CancellationTokenSource cts)
		{
			await Task.Run(() =>
			{
				if (_ipepToSessionId.TryGetValue(remoteIPEndPoint, out var sessionId))
				{
					_sessions[sessionId].Timestamp = DateTime.Now;
					// UDPは送信者1人に対して1つの最新情報を持つため、追加/上書きする
					_receiveDictUDP.AddOrUpdate(
						sessionId,
						(currentData) => data,
						(currentData, oldData) => data);
				}
				else
				{
					Log.Error("ReceiveEnqueue from IPEP でIPEPと sessionIdの変換に失敗");
				}
			});
		}

		/// <summary>
		/// クライアントからの受信キューからDequeue
		/// </summary>
		/// <param name="sessionId"></param>
		/// <param name="cts"></param>
		/// <returns></returns>
		//public async Task<ReceiveData> ReceiveDequeue(int sessionId, CancellationTokenSource cts)
		//{
		//	return await _sessions[sessionId].ReceiveQueue.Reader.ReadAsync(cts.Token);
		//}

		/// <summary>
		/// クライアントからの受信キューから試しにDequeueする
		/// </summary>
		/// <param name="sessionId"></param>
		/// <param name="data"></param>
		/// <returns></returns>
		public bool TryDequeue(int sessionId, out ReceiveData data)
		{
			return _sessions[sessionId].ReceiveQueue.Reader.TryRead(out data);
		}

		/// <summary>
		/// クライアントからのUDP受信キューから試しにDequeueする
		/// </summary>
		/// <param name="sessionId"></param>
		/// <param name="data"></param>
		/// <returns></returns>
		public bool TryDequeueUDP(int sessionId, out ReceiveData data)
		{
			return _receiveDictUDP.TryRemove(sessionId, out data);
			//return _sessions[sessionId].ReceiveQueue.Reader.TryRead(out data);
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
