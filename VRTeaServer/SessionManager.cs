using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer
{
	internal class SessionManager
	{
		public Action<IPEndPoint, int> OnDisconnected { get; }
		readonly ConcurrentDictionary<int Session>


		public SessionManager() { }


	}
}
