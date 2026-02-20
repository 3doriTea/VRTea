using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServerLib.Service
{
	public class GameLogic : IService
	{
		private SessionManager _sessionManager;

		public GameLogic(SessionManager sessionManager)
		{
			_sessionManager = sessionManager;
		}

		public Task Start(CancellationTokenSource cts)
		{
			throw new NotImplementedException();
		}
	}
}
