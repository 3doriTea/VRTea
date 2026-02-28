using System.Buffers.Binary;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Channels;

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
			int size = Encoding.UTF8.GetByteCount(str) + 5 + sizeof(int);  // null文字分追加


			byte[] buffer = new byte[size];
			// ビッグエンディアン(ネットワーク)として書き込み
			BinaryPrimitives.WriteInt32BigEndian(buffer, size);

			//MemoryMarshal.Write(buffer.AsSpan(0), in size);
			Encoding.UTF8.GetBytes(str).AsSpan().CopyTo(buffer.AsSpan(sizeof(int)));

			sendData = new SendData(buffer);
		}

		/// <summary>
		/// 送り先のIPEndPointを付け加える
		/// </summary>
		/// <param name="toIPEndPoint"></param>
		/// <returns></returns>
		public SendDataWithIPEP WithIPEP(IPEndPoint toIPEndPoint)
		{
			return new SendDataWithIPEP(Buffer, toIPEndPoint);
		}

		/// <summary>
		/// 文字列を取得
		/// </summary>
		/// <returns>文字列</returns>
		public readonly string GetString()
		{
			return Encoding.UTF8.GetString(Buffer.AsSpan(sizeof(int)));
		}
	}

	/// <summary>
	/// IPEndPointをくっつけた送信データ
	/// </summary>
	readonly struct SendDataWithIPEP
	{
		public readonly byte[] Buffer { get; }
		public IPEndPoint To { get; }  // 送る先のIPEP

		public SendDataWithIPEP(byte[] buffer, IPEndPoint to)
		{
			Buffer = buffer;
			To = to;
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
		public readonly string GetString()
		{
			return Encoding.UTF8.GetString(Buffer.AsSpan(sizeof(int)));
		}
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
		public Channel<ReceiveData> ReceiveQueue { get; } = Channel.CreateUnbounded<ReceiveData>();

		public bool ToDestroyFlag { get; set; } = false;
		public DateTime Timestamp { get; set; } = DateTime.UtcNow;

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
