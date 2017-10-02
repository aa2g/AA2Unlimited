powershell -Command "(new-object System.Net.WebClient).DownloadFile('https://github.com/crosire/reshade-shaders/archive/master.zip', 'shaders.zip')"
powershell -Command "$shell_app=new-object -com shell.application;$shell_app.namespace((Get-Location).Path + '\Shaders').Copyhere($shell_app.namespace((Get-Location).Path + '\shaders.zip\reshade-shaders-master\Shaders').items())"
powershell -Command "$shell_app=new-object -com shell.application;$shell_app.namespace((Get-Location).Path + '\Textures').Copyhere($shell_app.namespace((Get-Location).Path + '\shaders.zip\reshade-shaders-master\Textures').items())"

