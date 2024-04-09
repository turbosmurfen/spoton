<p align="center">
  <img width="120" height="120" src="https://raw.githubusercontent.com/turbosmurfen/spoton/main/img/spoton_logo.png">
</p>

[![Github All Releases](https://img.shields.io/github/downloads/turbosmurfen/spoton/total.svg)]() [![CodeQL](https://github.com/turbosmurfen/spoton/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/turbosmurfen/spoton/actions/workflows/codeql-analysis.yml)

# Spoton - Spotify Now Playing for mIRC  

![Spoton on mIRC 7.67](https://raw.githubusercontent.com/turbosmurfen/spoton/main/img/spoton_example.png)

Spoton running on mIRC 7.67

# Requirements

* **Supported Operating System**: Windows 10 and 11.
* **Visual C++ Redistributable 2015-2022 (x86)**  
* **Tested mIRC version**: 7.61 - 7.76  
* **Tested Spotify Version**: 1.1.91 - 1.2.33
* **HDD/SSD Space**: 18,5 KB.  

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
6. Navigate to the folder where you extracted the archive and select `spoton.dll`. 

# Usage

Use: _$dll(pathtospoton\spoton.dll,**command**,)_

| Command       | Description   |   
| --- | --- |  
| version       | Version of the plugin |  
| creator       | Credits of the plugin creator |  
| status        | 0 = Spotify not running, 1 = Spotify paused, 2 = Spotify playing an advertisement, 3 = Spotify playing a track. |
| song          | The entire window title, usually "artist - song title" |
| artist        | If the title contains a dash ("-"), returns the left side, usually the artist. Otherwise, nothing. |
| title         | If the title contains a dash ("-"), returns the right side, usually the song title. Otherwise, nothing. |

### Controlling Spotify from mIRC

Use: _/dll pathtospoton\spoton.dll control **command**_

| Command       |Description   |
| --- | --- | 
| playpause | Plays or Pauses the track. |
| play | Plays the track. |
| pause | Pauses the track. |
| next | Play next track. |
| previous | Play previous track. |
| forward | Forward the current track. |
| rewind | Rewind the current track. |
| show | Open the Spotify Window. |

### Show current song as status
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
  if (%status == 3) say Spotify » $dll(spoton.dll,song,)
  else echo -ag Spotify is $replace(%status,0,Not Running,1,Paused,2,Playing Advertisement)
}
```

### Add playback buttons to interface
This script adds playback buttons to your chat interface.

1. Download and extract the archive "Spoton mIRC Addon.zip" 
3. Copy the contents and paste to mIRC. Ensure you have spoton.dll there as well (from releases).
4. Type ``/load -rs spoton.mrc`` in mIRC. The script will return that it is loaded.

Removal:

1. Type ``/unload -rs spoton.mrc`` in mIRC
2. Remove spoton.dll and the folder img.   

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

> **Q**: I am getting an error: _/echo: insufficient parameters_ or _Artist - ??? ?????? ???_ or nothing.
>
> **A**: You are using an outdated version of Spoton. Please update.

## Credits

I have learned more about making mIRC reading and writing for DLL-file from [Wikichip](https://en.wikichip.org/wiki/mirc/dynamic-link_library)  
Thanks to [@Westor](https://github.com/westor7) for helping me out with fixing vulnerables and other things in the "Spoton mIRC Addon".  
Thanks to [@moobsmc](https://github.com/moobsmc) for Stop Control for Spotify.  
Thanks to [@Madis0](https://github.com/Madis0) for helping me with the readme and other things.  
