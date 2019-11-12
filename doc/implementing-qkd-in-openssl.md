This is part 4 in a multi-part report describing how we implemented Quantum Key Distribution (QKD) in OpenSSL as part of the pan-European quantum Internet hackathon in Delft on 5 and 6 November 2019. See [the main page of this report](../README.md) for the other parts.

# Implementing QKD in OpenSSL

## The RIPE quantum Internet hackathon challenge.

TODO

## Two approaches to adding QKD support to OpenSSL.

TODO

## Approach 1: Hacking the existing engine-based extension mechanism for Diffie-Hellman.

TODO

## Approach 2: Introducing QKD as a new key exchange protocol.

TODO

## Hacking the OpenSSL Diffie-Hellman engine to add QKD.

This section describes in detail how we "hacked" the existing engine-based extension mechanism for Diffie-Hellman to add support for QKD in OpenSSL on top of the ETSI QKD API.

TODO

## The mock implementation of the ETSI QKD API.

This sections describes in details on we created a "mock" (i.e. fake) implementation of the ETSI QKD API that allows us to test OpenSSL QKD without using any quantum network (neither a real quantum network nor a simulated quantum network).

TODO

## How to build and run the code in this repository.

The code in the repository as been tested on Apple macOS and on Ubuntu Linux 18.04 LTS in an AWS instance.

TODO
