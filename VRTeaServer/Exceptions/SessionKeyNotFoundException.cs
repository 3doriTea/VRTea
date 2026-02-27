using System;

namespace VRTeaServer.Exceptions
{
	internal class SessionKeyNotFoundException : Exception
	{
		public SessionKeyNotFoundException()
		{
		}

		public SessionKeyNotFoundException(string message) : base(message)
		{
		}

		public SessionKeyNotFoundException(string message, Exception inner) : base(message, inner)
		{
		}
	}
}
