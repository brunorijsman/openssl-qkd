#!/usr/bin/python

# TODO: Fix intermittent test failure:
# Did not match pattern #1: Connection establish acknowledge \(SYN\+ACK\): server port 44330
# Maybe the sleep is not long enough?

"""Analyze the decoded tshark capture file tshark.out to determine whether a Diffie-Hellman exchange
was completed as expected. Exits with success (0) a correct Diffie-Hellman exchange was completed,
or exits with failure (1) if not. """

from __future__ import print_function

import re
import sys

PATTERNS = [

    # TCP connection establishment
    r"Connection establish request \(SYN\): server port 44330",                 # 0
    r"Connection establish acknowledge \(SYN\+ACK\): server port 44330",        # 1

    # Client->Server TLS Client Hello
    r"TLSv1 Record Layer: Handshake Protocol: Client Hello",                    # 2
    r"Cipher Suite: TLS_DHE_RSA_WITH_AES_128_GCM_SHA256",                       # 3

    # Server->Client TLS Server Hello
    r"TLSv1.2 Record Layer: Handshake Protocol: Server Hello",                  # 4
    r"Cipher Suite: TLS_DHE_RSA_WITH_AES_128_GCM_SHA256",                       # 5
    r"TLSv1.2 Record Layer: Handshake Protocol: Certificate",                   # 6
    r"RelativeDistinguishedName item \(id-at-organizationName=Example\)",       # 7
    r"TLSv1.2 Record Layer: Handshake Protocol: Server Key Exchange",           # 8
    r"Diffie-Hellman Server Params",                                            # 9
    r"p Length: 256",                                                           # 10
    r"p:",                                                                      # 11
    r"g Length: 256",                                                           # 12
    r"g:",                                                                      # 13
    r"Pubkey Length:",                                                          # 14
    r"Pubkey:",                                                                 # 15
    r"TLSv1.2 Record Layer: Handshake Protocol: Server Hello Done",             # 16

    # Client->Server: TLS Client Key Exchange
    r"TLSv1.2 Record Layer: Handshake Protocol: Client Key Exchange",           # 17
    r"Diffie-Hellman Client Params",                                            # 18
    r"Pubkey Length:",                                                          # 19
    r"Pubkey:",                                                                 # 20
    r"TLSv1.2 Record Layer: Change Cipher Spec Protocol: Change Cipher Spec",   # 21
    r"TLSv1.2 Record Layer: Handshake Protocol: Encrypted Handshake Message",   # 22

    # Server->Client: TLS New Session Ticket
    r"TLSv1.2 Record Layer: Handshake Protocol: New Session Ticket",            # 23
    r"TLSv1.2 Record Layer: Change Cipher Spec Protocol: Change Cipher Spec",   # 24
    r"TLSv1.2 Record Layer: Handshake Protocol: Encrypted Handshake Message",   # 25

    # Client<->Server at least one TLS Application Data
    r"TLSv1.2 Record Layer: Application Data Protocol: Application Data"        # 26
]

def match_pattern(file, pattern):
    """ Continue reading file line by line and look for regular expression pattern.
    Returns True if the pattern was found, False if not. """
    for line in file:
        if re.search(pattern, line):
            return True
    return False

def check_tshark_out():
    """ Return True if decoded packet capture file tshark.out contains a valid TLS Diffie-Hellman
    exchange, or False if not. """
    index = 0
    pattern = PATTERNS[index]
    with open("tshark.out") as file:
        while True:
            if match_pattern(file, pattern):
                index += 1
                if index >= len(PATTERNS):
                    return True
                pattern = PATTERNS[index]
            else:
                print("Did not match pattern #{}: {}".format(index, pattern), file=sys.stderr)
                return False

if __name__ == "__main__":
    print("Checking tshark output for correct Diffie-Helman exchange... ", end="", file=sys.stderr)
    if check_tshark_out():
        print("OK", file=sys.stderr)
        sys.exit(0)
    else:
        print("FAIL", file=sys.stderr)
        sys.exit(1)
