using VRTeaServer.CLI;

namespace VRTeaServer
{
	internal class Program
	{
		static void Main(string[] args)
		{
			var askIPAddress = new AskIPAddress();
			var askPortNumber = new AskPortNumber();
			var askReady = new AskReady();

			var askPlayer = new AskPlayer(
			[
				new AskLine(),
				askIPAddress,
				askPortNumber,
				askReady,
			]);

			while (!askReady.Ready)
			{
				askPlayer.Play();
			}

			Console.Write($"Booting...");
			Console.Write($"\"{askIPAddress.IPAddress}:{askPortNumber.PortNumber}\"...");
			Console.WriteLine($"Ok!");
		}
	}
}
