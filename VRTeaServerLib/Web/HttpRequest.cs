using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServerLib.Web
{
	public class HttpRequest
	{
		public string Method { get; set; } = "";
		public string Directory { get; set; } = "";
		public string HTTPVersion { get; set; } = "";
		public byte[]? Content { get; set; } = null;

		public static HttpRequest Load(Span<byte> request)
		{
			int index = 0;
			index = request.IndexOf((byte)' ');
			request.Slice(0, index);

			string[] parts = request.Split(' ');
			int contentBegin = request.IndexOf("\r\n\r\n") + 4;
			return new HttpRequest { Method = parts[0], Directory = parts[1], HTTPVersion = parts[2], Content = request[contentBegin..] };
		}
	}
}
