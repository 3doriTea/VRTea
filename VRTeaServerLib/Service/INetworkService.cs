using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServerLib.Service
{
	public interface INetworkService
	{
		public TcpListener Listener { get; }
		public SessionManager.Session.Mode SessionMode { get; }

		public Task SessionClientAsync(int id, CancellationTokenSource cts);
	}
}
