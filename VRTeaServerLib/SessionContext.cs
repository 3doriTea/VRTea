using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServerLib
{
	public sealed class SessionContext
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

		public class Session : IDisposable
		{
			public int Id { get; }
			public TcpClient Client { get; }
			private CancellationTokenSource _cts { get; }

			public ConcurrentQueue<ReceiveData> ReceiveQueue { get; } = [];
			public ConcurrentQueue<ReceiveData> SendQueue { get; } = [];
			public bool ToDestroyFlag { get; set; } = false;
			public DateTime Timestamp { get; set; } = new();

			public Session(TcpClient client, int id, CancellationTokenSource cts)
			{
				Client = client;
				Id = id;
				_cts = cts;
			}

			public void Dispose()
			{
				_cts.Cancel();
				Client.Close();
				Client.Dispose();
			}
		}

		public SessionContext()
		{
			
		}
	}
}
