using VRTeaServer.CLI;
using VRTeaServer.Service;

namespace VRTeaServer
{
	internal class Program
	{
		const string LogDirectory = "./";
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

			var sessionManager = new SessionManager();

			var cts = new CancellationTokenSource();
			var servicePlayer = new ServicePlayer(
			[
				new LoggerService(new DirectoryInfo(LogDirectory)),
				new TerminateService(cts),
				new BotService(),
				new GameTcpService(
					sessionManager,
					askIPAddress.IPAddress,
					askPortNumber.PortNumber),

			]);

			servicePlayer.Play(cts);

			Console.Write("Closing server...");
			Console.WriteLine("Ok!");
		}
	}
}
