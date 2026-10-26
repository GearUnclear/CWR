import os,signal,subprocess
rows=[tuple(map(int,l.split())) for l in subprocess.check_output(['ps','-eo','pid=,ppid='],text=True).splitlines()]
def children(pid):
 for child,parent in rows:
  if parent==pid:
   yield from children(child)
   yield child
for pid,_ in rows:
 try: args=open('/proc/%s/cmdline'%pid,'rb').read().split(b'\0')
 except FileNotFoundError: continue
 if os.path.basename(args[0])==b'cmake' and b'--preset' in args and b'linux-x64-clang-rwdi' in args:
  for target in [*children(pid),pid]:
   try: os.kill(target,signal.SIGTERM)
   except ProcessLookupError: pass
