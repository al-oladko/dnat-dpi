## Overview

For DPI engine to identify L7 protocol, which operates through TCP, a connection between client and server must be established. Therefore, a three-way handshake are usually passed through the firewall. 
This method prevents the three-way handshake packets from being passed  until DPI determines L7 protoco. In order to prevent syn packet passing into an external network, synproxy technology was used. A data packet goes to DPI for analysis. If authorised L7 protocol is identified, the modified synproxy establishes a connection with the server. Data packets are accepted.
```
      TCP A                                                    FW                                   TCP B


  1. SYN-SENT    --> <SEQ=100><CTL=SYN>                       --> SYN-RECEIVED

  2. SYN-SENT    <-- <SEQ=400><ACK=101><CTL=SYN,ACK><WIN=100> <-- SYN-RECEIVED

  3. ESTABLISHED --> <SEQ=101><ACK=401><CTL=ACK>              --> ESTABLISHED

  4. SYN-SENT    --> <SEQ=101><ACK=401><DATA=AAAAA..>         --> DROP
```

Steps to allow TCP flow
```
      TCP A                                                    FW                                         TCP B


  1. SYN-SENT    --> <SEQ=100><CTL=SYN>                       --> SYN-RECEIVED

  2. SYN-SENT    <-- <SEQ=400><ACK=101><CTL=SYN,ACK><WIN=100> <-- SYN-RECEIVED

  3. ESTABLISHED --> <SEQ=101><ACK=401><CTL=ACK>              --> ESTABLISHED

  4. ESTABLISHED --> <SEQ=101><ACK=401><DATA=GET / HTTP...>   --> DROP

  5.                                                         SYN-SENT --> <SEQ=100><CTL=SYN>              --> SYN-RECEIVED

  6.                                                         SYN-SENT <-- <SEQ=500><ACK=101><CTL=SYN,ACK> <-- SYN-RECEIVED

  7.                                                      ESTABLISHED --> <SEQ=101><ACK=501><CTL=ACK>     --> ESTABLISHED

  8. ESTABLISHED <-- <SEQ=401><ACK=101><CTL=SYN,ACK><WIN=...> <-- Window Update

  9. (duplicate) --> <SEQ=101><ACK=401><DATA=GET / HTTP...>   --> SYNPROXY --> <SEQ=101><ACK=501><DATA=GET...> -->
```
The first data packet is dropped in step 4. Synproxy updates TCP window in step 8. This forces the client to retransmit packet #4 due to ack of packet #3. Therefore, there is no need to store packet #4. 

NAT is not supported yet.

## Installation

1. Copy iptables extensions
```
# cp extensions/libxt_spstate.c <patch to iptables source>/extensions
```
2. Build and install iptables.
3. Build module
```
# cd src
# build
```
4. Load module
```
# insmod ipt_SYNPROXY.ko
```

## Usage
Rule
```
iptables -A OUTPUT -m conntrack --ctstate ESTABLISHED -j ACCEPT
```
must be changed to
```
iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -m spstate ! --in-progress -j ACCEPT
```
Then 2 rules should be added for each DPI rule, that allows a flow.
```
iptables -A FORWARD "some other condition + dpi condition" -m spstate --in-progress -j SYNPROXY --sack-perm --timestamp --wscale 7 --mss 1460
iptables -A FORWARD "some other condition" -m spstate --none -j SYNPROXY --sack-perm --timestamp --wscale 7 --mss 1460
```

## Example
1. Build nfq.c and run it
```
# cd example
# gcc -o nfq nfq.c -lnetfilter_queue
# ./nfq
```
This program simulates DPI. If "GET " is found in the TCP stream, the stream will be identified as HTTP. It marks HTTP as 0x0b.

2. Load iptables rules
```
# iptables-restore < synproxy.rules
```
It should be noted that destination port 80 is checked in the rules.

3. Make request via FW

