#! /usr/bin/env tclsh
package require Tcl 8.5
package require sha256

# Semi-generic program for testing abuhops.

set LOCALPORT [expr {0+0xABCD}]
set SERVER localhost
set SERVERPORT 12545

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
  catch {
    exec -ignorestderr \
        << $data nc -w 1 -p $::LOCALPORT -u $::SERVER $::SERVERPORT
  }
}

proc udp-recv {} {
  if {[catch {
    puts begin
    set data [exec -ignorestderr -keepnewline \
                  << {} nc -u -l -w 5 -p $::LOCALPORT]
    puts done
  }]} {
    puts "No response."
    return no
  }

  binary scan $data c type
  set data [string range $data 1 end]
  if {![dict exists $::FORMATS $type]} {
    set name UNKNOWN
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

proc connect {id name} {
  set now [expr {[clock seconds] & 0xFFFFFFFF}]
  set idb [binary format i $id]
  set nowb [binary format i $now]
  set hmac [::sha2::hmac -bin -key $::SHARED_SECRET \
                "$idb$nowb$name"]
  udp-xmit "\x00$idb$nowb$hmac$name\x00"
  udp-recv
}
