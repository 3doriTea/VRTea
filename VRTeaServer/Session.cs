using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Channels;
using System.Threading.Tasks;

namespace VRTeaServer
{
	/// <summary>
	/// 結んでいるセッションのモード
	/// </summary>
	internal enum SessionMode
	{
		Game,  // ゲームの通信
	}

	/// <summary>
	/// 送信するデータ
	/// </summary>
	readonly struct SendData
	{
		public readonly byte[] Buffer { get; }

		public SendData(byte[] buffer)
		{
			Buffer = buffer;
		}

		/// <summary>
		/// 文字列から送信データに変換する
		/// </summary>
		/// <param name="str">文字列</param>
		/// <param name="sendData">送信データの out参照</param>
		public static void FromString(string str, out SendData sendData)
		{
			sendData = new SendData(Encoding.UTF8.GetBytes(str));
		}
	}

	/// <summary>
	/// 受信されたデータ
	/// </summary>
	readonly struct ReceiveData
	{
		public readonly byte[] Buffer { get; }
		public ReceiveData(byte[] buffer)
		{
			Buffer = buffer;
		}

		/// <summary>
		/// 文字列を取得
		/// </summary>
		/// <returns>文字列</returns>
		public readonly string GetString() => Encoding.UTF8.GetString(Buffer);
	}

	/// <summary>
	/// セッション
	/// </summary>
	internal class Session : IDisposable
	{
		public int Id { get; }
		public TcpClient Client { get; }
		public SessionMode Mode { get; }

		public Channel<SendData> SendQueue { get; } = Channel.CreateUnbounded<SendData>();
		public ConcurrentQueue<ReceiveData> ReceiveQueue { get; } = [];

		public bool ToDestroyFlag { get; } = false;
		public DateTime Timestamp { get; } = DateTime.UtcNow;

		private CancellationTokenSource _cts;

		public Session(TcpClient client, int id, SessionMode mode, CancellationTokenSource cts)
		{
			Client = client;
			Id = id;
			Mode = mode;

			_cts = cts;
		}


		public void Dispose()
		{
			Client.Close();
			Client.Dispose();
		}
	}
}
