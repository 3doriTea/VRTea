using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VRTeaServer.Service
{
	/// <summary>
	/// サービスに共通するインタフェース
	/// </summary>
	internal interface IService
	{
		/// <summary>
		/// 非同期処理開始処理
		/// </summary>
		/// <param name="cts">キャンセルトークン</param>
		/// <returns>非同期タスク</returns>
		public Task Start(CancellationTokenSource cts);
	}
}
