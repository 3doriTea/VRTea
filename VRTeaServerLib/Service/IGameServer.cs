using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServerLib.Service
{
	public interface IGameServer : INetworkService
	{
		void OnReceive(int id, );
	}
}
