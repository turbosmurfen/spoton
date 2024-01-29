;Welcome to Spoton mIRC Addon. Beta 0.0.2.
;This addon works best with Spoton 1.1.8 or later.

menu menubar,channel,query {
  -
  Spoton
  .Interface:spssh | dialog -m sps sps
  .-
  .Control
  ..Play/Pause:/dll spoton.dll control playpause
  ..Play:/dll spoton.dll control play
  ..Pause:/dll spoton.dll control pause
  ..Replay:/dll spoton.dll control replay
  ..Next:/dll spoton.dll control next
  ..Previous:/dll spoton.dll control previous
  ..Rewind:/dll spoton.dll control rewind
  ..Forward:/dll spoton.dll control forward
  .$iif($menu == channel,query)-
  .$iif($menu == channel,query)say:snp
}

dialog sps {
  title "SpotOn"
  size -1 -1 187 64
  option dbu
  box "", 1, 1 0 185 53
  edit "", 2, 3 39 180 11, autohs return
  icon 3, 4 53 180 12,  spssh.bmp, 0, noborder
  text "", 4, 5 7 111 30
  button "Save", 5, 152 7 29 10, flat disable
  button "List", 6, 152 21 29 10, flat disable
}

on *:dialog:sps:*:*:{
  if ($devent == init) {
    did -a $dname 4 $spfrmx
    if ($lines(says.txt) > 0) { did -e $dname 6 }
    if (%saythis) {
      spssh %saythis 
      did -ra $dname 2 %saythis | did -e $dname 5
    }
  }
  if ($devent == sclick) {
    if ($did == 5) {
      if (!$read(says.txt, nw, * $+ $did(2))) {
        set %saythis $did(2)
        var %line = $iif($lines(says.txt) == 0,1,$calc($v1 +1))
        write says.txt %line $+ $chr(144) $+ $did(2)
      }
      else { noop $input(The Say is already in the list!,woud,SpotOn) | set %saythis $did(2) }
      did -e $dname 6
    }
    if ($did == 6) { $iif($window(@saylist),window -c @saylist,spotwin) }
    if ($did == 3) { did -r $dname 2 | spssh }
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
    var %ttext = $1-
    var %tfont = tahoma, %tsize = 11
    window -dBk0pw0h +dL @spss -1 -1 360 15
    drawfill -r @spss $rgb(face) $rgb(face) 0 0
    drawrect -rf @spss $rgb(face) 1 $calc($width(%ttext,%tfont,%tsize,0,1) + 3) 0 360 15
    drawtext -pb @spss $color(text) $color(background) %tfont %tsize 2 1 %ttext
    drawrect -r @spss $color(text) 1 0 0 $calc($width(%ttext,%tfont,%tsize,0,1) + 3) 15
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

;Add new features from SpotOn
alias -l spfrmx { 
  return $+([song],$str($chr(9),2),Shows the Artist - Title,$crlf,[artist],$str($chr(9),2),Shows the Artist,$crlf,[title],$str($chr(9),2),Shows the Title)
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
  else { set %saythis spotify > [song] }
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
      set %saythis $gettok($sline(@saylist,1),2,160)
    }
  }
  Remove:write -dl $+ $sline(@saylists,1).ln says.txt | spotwin
}

;## 0ther ##


;Replace value with data from [x] [y] [z]
alias -1 spfrm {
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
alias spfind {
  if ($exists($+($nofile($script),spoton.dll))) {
    return $+($nofile($script),spoton.dll)
  }
  else { echo 04 -ag * [Spoton] - Can't find spoton.dll. | halt }
}

;If Spoton is playing a song (Status Code: 3), write out to the channel/pm.
alias snp {
  if ($dll($spfind,status,0) == 3) {
    if ($server != $null) {
      say $spfrm(%saythis)
    }
    else {
      echo 04 -ag * [Spoton] - You are not connected to an IRC-Server.
    }
  }
  else { echo 02 -ag * [Spoton] - Spotify is $replace($dll($spfind,status,0),0,Not running,1,Paused,2,Playing Advertisement) $+ . | halt }
}

;Setup everything that is needed when loaded.
on *:load:{
  if (!%saythis) { set %saythis Spotify » [song] }
  spssh
  echo 02 -ag * [Spoton] - Spoton is loaded!
}

on *:unload:{
  if ($dialog(sps)) { dialog -x sps }
  if ($isfile(spssh.bmp)) { .remove spssh.bmp }
  if ($window(@saylists)) { window -c $v1 }
  if ($window(@spss)) { window -c $v1 }
  if ($isfile(says.txt)) { .remove says.txt }
  unset %saythis
  echo 02 -ag * [Spoton] - SpotOn is now unloaded!
}
