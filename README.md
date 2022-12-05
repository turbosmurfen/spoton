<p align="center">
  <img width="120" height="120" src="https://github.com/turbosmurfen/spoton/blob/main/img/spoton_logo.png">
</p>

[![Github All Releases](https://img.shields.io/github/downloads/turbosmurfen/spoton/total.svg)]() [![CodeQL](https://github.com/turbosmurfen/spoton/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/turbosmurfen/spoton/actions/workflows/codeql-analysis.yml)

# Spoton - Spotify Now Playing for mIRC  

**Version 1.1.5 and newer fixes a memory leak, updating is highly recommended.**

![Spoton on mIRC 7.67](https://github.com/turbosmurfen/spoton/blob/main/img/spoton_example.png)

Spoton running on mIRC 7.67

# Requirements

* **Supported Operating System**: Windows Vista, 7, 8.1, 10 and 11.
* **Visual C++ Redistributable 2015-2022 (x86)**  
* **Tested mIRC version**: 7.61 >= 7.72  
* **Tested Spotify Version**: 1.1.91 >= 1.1.99
* **Harddrive Space**: 17,5 KB.  

# Installation 

1. Download `spoton_vx.x.x.zip` zip archive of Spoton in Assets in the [Releases](https://github.com/turbosmurfen/spoton/releases/latest) section.
2. If you don't have `Visual C++ Redistributable 2015-2022 (x86)` installed, download from here: [https://aka.ms/vs/17/release/vc_redist.x86.exe](https://aka.ms/vs/17/release/vc_redist.x86.exe).  
3. Right Click on the archive which is named _spoton_vx.x.x.zip_, extract the archive.  
   * Optional - verify the hash: 
       1. Open up powershell and **cd** to **spoton_vx.x.x** folder. 
       2. Run this command `Get-FileHash spoton.dll`. 
       3. Then look if the sha256 checksum is correct from [Releases](https://github.com/turbosmurfen/spoton/releases/latest). 
       4. If it's correct you should be fine. 
4. Open mIRC.
5. Paste `//noop $sfile($mircdir)` to mIRC and press enter. You will see a file picker dialog.
6. Copy `spoton.dll` and paste inside this popup window. **OR** save it where you have your other DLL files. 

# Usage

Use: _$dll(pathtospoton\spoton.dll,**command**,)_

| Command       | output        | Description   |   
| --- | --- | --- |  
| version       | x.x.x         | Will output which version of spoton you use.  |  
| creator       | x - Made by   | Will output the creator of spoton.  |  
| status        | 0             | Spotify is not running. |
| status        | 1             | Spotify is paused. |
| status        | 2             | Spotify is playing advertisement. |
| status        | 3             | Spotify is playing a song.
| song          | artist - title | Will output artist and title. |

### Controlling Spotify from mIRC

Use: _/dll pathtospoton\spoton.dll control **command**_

| Command       |Description   |
| --- | --- | 
| playpause | Plays or Pause a song. |
| stop | Stops the playing song. |
| replay | Play the song from the beginning. |
| next | Play next Spotify song. |
| previous | Play previous Spotify song. |
| forward | Forward the current track. |
| rewind | Rewind the current track. |
| volup | Increase the volume in Spotify. |
| voldown | Decrease the volume in Spotify. |
| volmute | Mute or Unmute Spotify volume. |

### Send current song to chat

Please ensure Spoton alias **snp** is not triggered by any other scripts.  
  
To add the script:
1. In mIRC, click on **Scripts Editor** or **ALT** + **R**
2. Select Remote.
3. Click on File > New.  
4. Copy the code below and paste inside the new Script file
5. Save. Now you can use `/snp` in any channel or private messages.

```mirc
alias snp {
  var %status $dll(spoton.dll,status,)
  if (%status == 1) echo -a Spotify is paused.
  elseif (%status == 2) echo -a Spotify is playing Advertisement.
  elseif (%status == 3) say Spotify » $dll(spoton.dll,song,)
  else echo -a Spotify is not running.
}
```

# Frequently Asked Questions

> **Q**: I can't find vx.x.x on download section, what do I do wrong?
> 
> **A**: **x.x.x** stands for the version number, look for that in releases.

> **Q**: How does Spoton work?
>
> **A**: It works by reading Windows API calls from Spotify to detect _artist - title_, plus using Spotify's media controls.

> **Q**: Does Spoton require a Spotify API key or internet connection?
>
> **A**: No. Spoton does not use Spotify API keys and it works offline.  

> **Q**: I am getting an error: _$dll: unable to open 'C:\Users\USERNAME\AppData\Roaming\mIRC\pathtospoton\spoton.dll_
>
> **A**: This can show up for 2 reasons:  
> 
> 1. You have put the DLL file in the wrong location.
> 2. You need to install [Visual C++ Redistributable 2015-2022 (x86)](https://aka.ms/vs/17/release/vc_redist.x86.exe)  

> **Q**: I am getting an error: _/echo: insufficient parameters_ or _Artist - ??? ?????? ???_.
>
> **A**: You are using an outdated version of Spoton. Please update.

## Credits

I have learned more about making mIRC reading and writing for DLL-file from [Wikichip](https://en.wikichip.org/wiki/mirc/dynamic-link_library)  
Thanks to [Westor](https://github.com/westor7) for helping me out with fixing vulnerables and other things in the mIRC Beta Addon for Spoton.  
