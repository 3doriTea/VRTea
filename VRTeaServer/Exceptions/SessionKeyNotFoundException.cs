using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.Exceptions
{
	internal class SessionKeyNotFoundException : Exception
	{
		public SessionKeyNotFoundException(string message) :
			base(message)
		{
		}
	}
}
