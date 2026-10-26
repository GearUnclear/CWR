# Runs a command line on a private (invisible) desktop so game windows never reach the owner's desktop.
# Scratch tool, untracked. Usage:
#   powershell -NoProfile -File D:\Arma_CWA\tmp\tri-hidden-desktop.ps1 -Log <logfile> -Cmd '<command line>' [-WorkDir <dir>] [-DesktopName udtest]
# Example:
#   ... -Log tmp\hd.log -WorkDir D:\Arma_CWA\arma_CWR -Cmd 'target\debug\tri.exe test --retries 0 -j1 --game-dir dist/agent-rwdi --data-dir D:/Arma_CWA/tmp/remaster-shadow-of5 tests/integration/scripting/ai_group_id_churn.test.sqf'
# Turning it off: just run tri directly. Exit code = the command's exit code.
param(
  [Parameter(Mandatory)][string]$Cmd,
  [Parameter(Mandatory)][string]$Log,
  [string]$WorkDir = 'D:\Arma_CWA\arma_CWR',
  [string]$DesktopName = ('udtest' + $PID),
  [int]$TimeoutSec = 900
)
Add-Type -TypeDefinition @'
using System; using System.Runtime.InteropServices;
public static class HD {
  [StructLayout(LayoutKind.Sequential, CharSet=CharSet.Unicode)]
  public struct STARTUPINFO { public int cb; public string lpReserved; public string lpDesktop; public string lpTitle;
    public int dwX, dwY, dwXSize, dwYSize, dwXCountChars, dwYCountChars, dwFillAttribute, dwFlags;
    public short wShowWindow, cbReserved2; public IntPtr lpReserved2, hStdInput, hStdOutput, hStdError; }
  [StructLayout(LayoutKind.Sequential)]
  public struct PROCESS_INFORMATION { public IntPtr hProcess, hThread; public int dwProcessId, dwThreadId; }
  [DllImport("user32.dll", CharSet=CharSet.Unicode, SetLastError=true)]
  public static extern IntPtr CreateDesktopW(string name, IntPtr dev, IntPtr devmode, int flags, uint access, IntPtr sa);
  [DllImport("user32.dll", SetLastError=true)] public static extern bool CloseDesktop(IntPtr h);
  [DllImport("kernel32.dll", CharSet=CharSet.Unicode, SetLastError=true)]
  public static extern bool CreateProcessW(string app, string cmd, IntPtr pa, IntPtr ta, bool inherit, uint flags,
    IntPtr env, string cwd, ref STARTUPINFO si, out PROCESS_INFORMATION pi);
  [DllImport("kernel32.dll")] public static extern uint WaitForSingleObject(IntPtr h, uint ms);
  [DllImport("kernel32.dll")] public static extern bool GetExitCodeProcess(IntPtr h, out uint code);
  [DllImport("kernel32.dll")] public static extern bool TerminateProcess(IntPtr h, uint code);
  [DllImport("kernel32.dll")] public static extern bool CloseHandle(IntPtr h);
}
'@
$GENERIC_ALL = 0x10000000
$desk = [HD]::CreateDesktopW($DesktopName, [IntPtr]::Zero, [IntPtr]::Zero, 0, $GENERIC_ALL, [IntPtr]::Zero)
if ($desk -eq [IntPtr]::Zero) { throw "CreateDesktopW failed: $([Runtime.InteropServices.Marshal]::GetLastWin32Error())" }
try {
  $si = New-Object HD+STARTUPINFO
  $si.cb = [Runtime.InteropServices.Marshal]::SizeOf($si)
  $si.lpDesktop = "WinSta0\$DesktopName"
  $pi = New-Object HD+PROCESS_INFORMATION
  $cmdline = "`"$env:ComSpec`" /c `"$Cmd > $Log 2>&1`""
  $ok = [HD]::CreateProcessW($env:ComSpec, $cmdline, [IntPtr]::Zero, [IntPtr]::Zero, $false, 0x08000000, [IntPtr]::Zero, $WorkDir, [ref]$si, [ref]$pi)
  if (-not $ok) { throw "CreateProcessW failed: $([Runtime.InteropServices.Marshal]::GetLastWin32Error())" }
  Write-Host "started pid $($pi.dwProcessId) on desktop WinSta0\$DesktopName"
  $w = [HD]::WaitForSingleObject($pi.hProcess, [uint32]($TimeoutSec * 1000))
  if ($w -ne 0) { Write-Host 'timeout'; & taskkill /PID $pi.dwProcessId /T /F | Out-Null; $code = 124 }
  else { $c = 0; [void][HD]::GetExitCodeProcess($pi.hProcess, [ref]$c); $code = [int]$c }
  [void][HD]::CloseHandle($pi.hProcess); [void][HD]::CloseHandle($pi.hThread)
  Write-Host "exit $code"
  exit $code
} finally { [void][HD]::CloseDesktop($desk) }
