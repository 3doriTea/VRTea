using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.Service
{
	/// <summary>
	/// サービスをまとめて再生するやつ
	/// </summary>
	internal class ServicePlayer
	{
		private List<IService> _services;
		public ServicePlayer(List<IService> services)
		{
			_services = services;
		}

		/// <summary>
		/// 再生する
		/// </summary>
		/// <param name="cts">キャンセルトークン</param>
		public void Play(CancellationTokenSource cts)
		{
			var tasks = new List<Task>();
			foreach (var service in _services)
			{
				tasks.Add(service.Start(cts));
			}

			Task.WhenAll(tasks).Wait();
		}
	}
}
