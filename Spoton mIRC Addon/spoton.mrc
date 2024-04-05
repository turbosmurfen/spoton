;Welcome to Spoton mIRC Addon. Beta 0.0.6.
;This addon works best with Spoton 1.2.0 or later.

;Icons made by icons8 - https://icons8.com/

menu menubar,channel,query,status {
  -
  Spoton
  .Interface:spssh | dialog -m sps sps
  .-
  .$iif($menu == channel,Now Playing):/snp
  .$iif($menu == query,Now Playing):/snp
  .$iif($dll($spfind,status,) != 0,Open Spotify):/dll spoton.dll control show
  .$iif($dll($spfind,status,) == 0,Start Spotify):/sptnrun

}

dialog sps {
  title "Spoton"
  size -1 -1 196 109
  option dbu
  box "Say messages", 1, 3 3 189 67
  edit "", 2, 7 41 180 11, return autohs
  ;icon 3, 7 53 183 15,  spssh.bmp, 0, noborder
  icon 3, 7 53 183 15,  spssh.bmp, 0, noborder
  text "", 4, 8 12 140 27
  button "Save", 5, 152 7 29 10, disable flat
  button "List", 6, 152 21 29 10, disable flat
  radio "Disabled", 7, 8 83 34 10
  radio "Extended", 8, 44 83 38 10
  radio "Compact", 9, 85 83 38 10
  box "Media controls", 11, 3 72 189 27
  link "Media Icons by icons8", 10, 3 99 56 8
}

on *:dialog:sps:*:*:{
  if ($devent == init) {
    did -a $dname 4 $spfrmx

    if (%sptnmedia == 0) did -c $dname 7
    if (%sptnmedia == 1) did -c $dname 8
    if (%sptnmedia == 2) did -c $dname 9
    dialog -t $dname $+(Spoton v,$dll($spfind,version,),$chr(32),- Addon Beta: %sptnbeta)
    if ($lines(says.txt) > 0) { did -e $dname 6 }
    if (%sptnsaythis) {
      spssh %sptnsaythis
      did -ra $dname 2 %sptnsaythis | did -e $dname 5
    }
  }
  if ($devent == sclick) {
    if ($did == 5) {
      if (!$read(says.txt, nw, * $+ $did(2))) {
        set %sptnsaythis $did(2)
        var %line = $iif($lines(says.txt) == 0,1,$calc($v1 +1))
        write says.txt %line $+ $chr(144) $+ $did(2)
      }
      else { noop $input(The Say is already in the list!,woud,Spoton) | set %sptnsaythis $did(2) }
      did -e $dname 6
    }
    if ($did == 6) { $iif($window(@saylist),window -c @saylist,spotwin) }
    if ($did == 3) { did -r $dname 2 | spssh }
    if ($did == 7) { set %sptnmedia 0 | sptntb 0 }
    if ($did == 8) { set %sptnmedia 1 | sptntb 1 }
    if ($did == 9) { set %sptnmedia 2 | sptntb 2 }
  }
  if ($devent == edit) {
    if ($did == 2) {
      if ($len($did(2)) > 0) { spssh $did(2) }
      if ($len($did(2)) >= 5) { did -e $dname 5 }
      elseif ($len($did(2)) < 5) { did -b $dname 5 }
      elseif ($len($did(2)) == 0) { spssh }
      did -f $dname 2
    }
  }
  if ($devent == close) { window -c @spss | window -c @saylist }
}

;Generate Image into the tool
alias -l spssh {
  if ($1- != $null) {
    clear @spss
    var %ttext = $replace($1-,[song],Artist - Title,[artist],Artist,[title],Title)
    var %tfont = $window(status window).font, %tsize = $window(status window).fontsize
    window -dBk0pw0h +dL @spss -1 -1 360 $calc($height(%ttext,%tfont,%tsize,0,1) + 3)
    drawfill -r @spss $rgb(face) $rgb(face) 0 0
    drawrect -rf @spss $rgb(face) 1 $calc($width(%ttext,%tfont,%tsize,0,1) + 3) 0 360 $calc($height(%ttext,%tfont,%tsize,0,1) + 3)
    drawtext -pb @spss $color(text) $color(background) %tfont %tsize 2 1 %ttext
  }
  else {
    clear @spss
    window -dBk0pw0h +dL @spss -1 -1 360 15
    drawrect -rf @spss $rgb(face) 1 0 0 360 15
  }
  drawsave @spss spssh.bmp
  $iif($dialog(sps),did -g sps 3 spssh.bmp)
  window -c @spss
}

;This will show which features you can use in Spoton
alias -l spfrmx { 
  return $+([song],$str($chr(9),2),Shows the Artist - Title,$crlf,[artist],$str($chr(9),2),Shows the Artist,$crlf,[title],$str($chr(9),2),Shows the Title)
}

;Run Spotify if it's not running
alias -l sptnrun {

  ;If spotify is downloaded from website.
  var %path1 = $+($sysdir(profile),AppData\Roaming\Spotify\Spotify.exe)

  ;If spotify is downloaded from Windows apps.
  var %path2 = $+($sysdir(profile),AppData\Local\Microsoft\WindowsApps\Spotify.exe)

  if ($dll(spoton.dll,status,) == 0) {
    if ($isfile(%path1)) {
      /run %path1
    }
    elseif ($isfile(%path2)) {
      /run %path2
    }
    else {
      echo 04 -ag * [Spoton] - Spotify is not installed on this system.
    }
  }
  else {
    echo 04 -ag * [Spoton] - Spotify is already running.
  }
}
;Generate says into the window
alias -l spotwin {
  if ($lines(says.txt) > 0) {
    clear @saylist
    var %x = $iif($dialog(sps),$calc($dialog(sps).x + $dialog(sps).w),-1)
    var %y = $iif($dialog(sps),$dialog(sps).y,-1)
    window -ak0ld $+ $iif($dialog(sps) == $null,C) +L @saylist %x %y 200 200
    var %o = 1
    while (%o <= $lines(says.txt)) {
      aline @saylist $+($chr(2),%o,.,$chr(2),$chr(160),$gettok($read(says.txt,n,%o),2,144))
      inc %o
    }
  }
  else { set %sptnsaythis Spotify » [song] }
}

;Select and Remove says.
menu @saylist {
  dclick:{
    if ($sline(@saylist,1)) {
      if ($dialog(sps)) {
        did -ra sps 2 $gettok($sline(@saylist,1),2,160)
        spssh $gettok($sline(@saylist,1),2,160)
        did -e sps 5
      }
      set %sptnsaythis $gettok($sline(@saylist,1),2,160)
    }
  }
  Remove:write -dl $+ $sline(@saylists,1).ln says.txt | spotwin
}

;## 0ther ##


;Replace value with data from [song], [artist], [title]
alias -l spfrm {
  return $spc($replace($1-,[song],$dll($spfind,song,),[artist],$dll($spfind,artist,),[title],$dll($spfind,title,)))
}

;Check if channel have +c (Colors enabled)
alias -l spc {
  if ($left($active,1) == $chr(35)) {
    if (c !isincs $chan(#).mode) { return $1- }
    else { return $strip($1-) }
  }
  else { return $1- }
}

;Checks if spoton.dll exists and return the path
alias -l spfind {
  if ($exists($+($nofile($script),spoton.dll))) {
    return $+($nofile($script),spoton.dll)
  }
  else { echo 04 -ag * [Spoton] - Can't find spoton.dll. | halt }
}

;If Spoton is playing a song (Status Code: 3), write out to the channel/pm.
alias snp {
  if ($dll($spfind,status,0) == 3) {
    if ($server != $null) {
      if ($left($active,1) == $chr(35) || $active != Status Window) {
        say $spfrm(%sptnsaythis)
      }
    }
    else {
      echo 04 -ag * [Spoton] - You are not connected to an IRC-Server.
    }
  }
  else { echo 02 -ag * [Spoton] - Spotify is $replace($dll($spfind,status,0),0,Not running,1,Paused,2,Playing Advertisement) $+ . | halt }
}

;Reset the toolbar buttons
alias -l sptnresets {
  if ($toolbar(sepa1)) toolbar -d sepa1
  if ($toolbar(sptn-previous)) toolbar -d sptn-previous
  if ($toolbar(sptn-rewind)) toolbar -d sptn-rewind
  if ($toolbar(sptn-play)) toolbar -d sptn-play
  if ($toolbar(sptn-forward)) toolbar -d sptn-forward
  if ($toolbar(sptn-next)) toolbar -d sptn-next
}

;Generate toolbar buttons
alias -l sptntb {
  if ($isnum($1)) {
    var %a %sptncntrl
    var %i 1
    var %b %sptncntrl + 7
    if ($1 == 0) { 
      sptnresets
    }
    else {
      while (%a <= %b) {
        if (%i == 1) {
          sptnresets
          if ($toolbar(%a).name == $null) toolbar -izs %a sepa1
        }
        if ($1 == 2) {
          if (%i == 2) {
            if ($toolbar(%a).name == $null) toolbar -iz1 %a sptn-previous "Previous track" img\previoustrack.ico "/dll spoton.dll control previous"
          }
          if (%i == 3) {
            if ($toolbar(%a).name == sptn-rewind) toolbar -d sptn-rewind
            if ($toolbar(%a).name == $null) toolbar -iz1 %a sptn-play "Play/Pause track" img\playpausetrack.ico "/dll spoton.dll control playpause"
          }
          if (%i == 4) {
            if ($toolbar(%a).name == $null) toolbar -iz1 %a sptn-next "Next track" img\nexttrack.ico "/dll spoton.dll control next"
          }
          if (%i == 5) {
            if ($toolbar(%a).name == sptn-forward) toolbar -d sptn-forward

          }
          if (%i == 6) {
            if ($toolbar(%a).name == sptn-next) toolbar -d %a
          }
        }
        if ($1 == 1) {
          if (%i == 2) {
            if ($toolbar(%a).name == $null) toolbar -iz1 %a sptn-previous "Previous track" img\previoustrack.ico "/dll spoton.dll control previous"
          }
          if (%i == 3) {
            if ($toolbar(%a).name == sptn-play) toolbar -d %a
            if ($toolbar(%a).name == $null) toolbar -iz1 %a sptn-rewind "Rewind track" img\rewindtrack.ico "/dll spoton.dll control rewind"
          }
          if (%i == 4) {
            if ($toolbar(%a).name == sptn-next) toolbar -d sptn-next
            toolbar -iz1 %a sptn-play "Play/Pause track" img\playpausetrack.ico "/dll spoton.dll control playpause"
          }
          if (%i == 5) {
            if ($toolbar(%a).name == $null) toolbar -iz1 %a sptn-forward "Forward track" img\forwardtrack.ico "/dll spoton.dll control forward"
          }
          if (%i == 6) {
            if ($toolbar(%a).name == $null) toolbar -iz1 %a sptn-next "Next track" img\nexttrack.ico "/dll spoton.dll control next"
          }
        }

        inc %a
        inc %i
      }
    }
  }
}

;Setup everything that is needed when loaded.
on *:load:{
  set %sptnsaythis Spotify » [song]
  spssh
  set %sptncntrl $toolbar(0) + 1
  set %sptnbeta 0.0.6
  set %sptnmedia 0
  echo 02 -ag * [Spoton] - Spoton is loaded!
}
;Automatic add toolbars if enabled
on *:start:{
  if (%sptnmedia > 0) sptntb %sptnmedia
}
on *:unload:{
  if ($dialog(sps)) { dialog -x sps }
  if ($isfile(spssh.bmp)) { .remove spssh.bmp }
  if ($window(@saylists)) { window -c $v1 }
  if ($window(@spss)) { window -c $v1 }
  if ($isfile(says.txt)) { .remove says.txt }
  unset %sptnsaythis
  unset %sptnbeta
  unset %sptncntrl
  unset %sptnmedia
  sptnresets
  echo 02 -ag * [Spoton] - Spoton is now unloaded!
}
