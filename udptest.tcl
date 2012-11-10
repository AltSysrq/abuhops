#! /usr/bin/env tclsh
package require Tcl 8.5
package require sha256
package require udp

# Semi-generic program for testing abuhops.

set LOCALPORT [expr {0+0xABCD}]
set SERVER 127.0.0.1
set SERVERPORT 12545

set SOCK [udp_open $LOCALPORT]
fconfigure $SOCK -buffering none -translation binary
fileevent $SOCK readable udp-recv
udp_conf $SOCK $SERVER $SERVERPORT

set FORMATS {
  00 { YOU-ARE address 4 port 2 }
  01 { PONG }
  02 { FROM-OTHER }
  03 { ADVERT id 4 address 4 port 2 }
  05 { SIGNATURE domain 4 signature 34 public-key 17 }
}

proc udp-xmit {data} {
  binary scan $data H* hex
  puts "Xmit: $hex"
  puts -nonewline $::SOCK $data
}

proc udp-recv {} {
  set data [read $::SOCK]

  binary scan $data c type
  set type [format %02X $type]
  set data [string range $data 1 end]
  if {![dict exists $::FORMATS $type]} {
    set name UNKNOWN:$type
    set fmt {}
  } else {
    set fmt [lassign [dict get $::FORMATS $type] name]
  }

  puts "Received $name:"
  foreach {label length} $fmt {
    set item [string range $data 0 $length-1]
    set data [string range $data $length end]
    binary scan $item H* hex
    puts "  $label: $hex"
  }
  binary scan $data H* hex
  puts "  tail: $hex\n"
  return yes
}

set f [open "test_secret" rb]
set SHARED_SECRET [read $f]
close $f

proc sleep {delay} {
  set ::sleepDone no
  after $delay [list set ::sleepDone yes]
  vwait ::sleepDone
}

proc connect {id name} {
  set now [expr {[clock seconds] & 0xFFFFFFFF}]
  set idb [binary format i $id]
  set nowb [binary format i $now]
  set hmac [::sha2::hmac -bin -key $::SHARED_SECRET \
                "$idb$nowb$name"]
  udp-xmit "\x00$idb$nowb$hmac$name\x00"
  sleep 5000
}

proc connect-invalid {id name} {
  set now [expr {[clock seconds] & 0xFFFFFFFF}]
  set idb [binary format i $id]
  set nowb [binary format i $now]
  set hmac [::sha2::hmac -bin -key "foo" "$idb$nowb$name"]
  udp-xmit "\x00$idb$nowb$hmac$name\x00"
  sleep 5000
}

proc ping {whoAmI} {
  set wai [binary format c $whoAmI]
  udp-xmit "\x01$wai"
  sleep 5000
}

proc post {data} {
  udp-xmit "\x03[binary format H* $data]"
  sleep 1000
}

proc lst {} {
  udp-xmit "\x04"
  sleep 5000
}

proc bye {} {
  udp-xmit "\x06"
  sleep 5000
}
