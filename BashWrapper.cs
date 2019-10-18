using System.Diagnostics;
using System.Threading;

public class BashWrapper
{
	static void Main(string[] args)
	{
		Process.Start("/bin/bash", "-c \"" + string.Join(" ", args) + "\"");
		Thread.Sleep(Timeout.Infinite);
	}
}

