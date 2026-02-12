using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Channels;
using System.Threading.Tasks;

namespace VRTeaServerLib
{
	public sealed class SessionManager
	{
		public readonly struct ReceiveData
		{
			public readonly byte[] buffer;

			public ReceiveData(byte[] bytes)
			{
				buffer = bytes;
			}

			public readonly string GetString()
			{
				return Encoding.UTF8.GetString(buffer);
			}
		}

		public readonly struct SendData
		{
			public readonly byte[] buffer;

			public SendData(byte[] bytes)
			{
				buffer = bytes;
			}

			public static SendData FromString(string str)
			{
				return new SendData(Encoding.UTF8.GetBytes(str));
			}
		}

		/// <summary>
		/// <para>クライアントとのセッション</para>
		/// <para>設計上、必ずTcpの socketはできている</para>
		/// </summary>
		public class Session : IDisposable
		{
			public enum Mode
			{
				Game,
			}

			public int Id { get; }
			public TcpClient Client { get; }
			public Mode SessionMode { get; }
			private CancellationTokenSource _cts { get; }

			public ConcurrentQueue<ReceiveData> ReceiveQueue { get; } = [];
			public Channel<SendData> SendQueue { get; } = Channel.CreateUnbounded<SendData>();
			public bool ToDestroyFlag { get; set; } = false;
			public DateTime Timestamp { get; set; } = new();

			public Session(TcpClient client, int id, CancellationTokenSource cts, Mode mode)
			{
				Client = client;
				Id = id;
				SessionMode = mode;
				_cts = cts;
			}

			public void Dispose()
			{
				_cts.Cancel();
				Client.Close();
				Client.Dispose();
			}
		}

		public Action<IPEndPoint, int> OnDisconnected = (IPEndPoint ipEndPoint, int sessionId) => { };
		private readonly ConcurrentDictionary<int, Session> _sessions = [];
		private readonly ConcurrentDictionary<IPEndPoint, int> _toSessionId = [];
		private int sessionIdCounter = 0;

		public SessionManager()
		{
		}

		/// <summary>
		/// セッションを開始する
		/// </summary>
		/// <param name="session"></param>
		/// <returns></returns>
		public int BeginSession(TcpClient tcpClient, CancellationTokenSource cts, Session.Mode sessionMode)
		{
			//int sessionId = Interlocked.CompareExchange(ref sessionIdCounter, 0, 0);
			int sessionId = Interlocked.Increment(ref sessionIdCounter);
			var session = new Session(tcpClient, sessionId, cts, sessionMode);

			if (tcpClient.Client.RemoteEndPoint is not IPEndPoint ipEndPoint)
			{
				Log.Error($"{tcpClient.Client.RemoteEndPoint} is not IPEndPoint");
				return 0;
			}

			_toSessionId.AddOrUpdate(
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

		public Session GetSession(int id)
		{
			return _sessions[id];
		}

		/// <summary>
		/// セッションを終了する
		/// </summary>
		/// <param name="id"></param>
		public void EndSession(int id)
		{
			_sessions.Remove(id, out _);
		}
	}
}
