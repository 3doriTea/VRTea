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
	internal class SessionManager
	{
		public Action<IPEndPoint, int> OnDisconnected { get; }
		private readonly ConcurrentDictionary<int, Session> _sessions;
		private readonly ConcurrentDictionary<IPEndPoint, int> _ipepToSessionId;
		private int _sessionIdCounter;

		public SessionManager() { }

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

		public void EndSession(int sessionId)
		{

		}
	}
}
