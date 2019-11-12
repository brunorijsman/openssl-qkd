This is part 4 in a multi-part report describing how we implemented Quantum Key Distribution (QKD) in OpenSSL as part of the pan-European quantum Internet hackathon in Delft on 5 and 6 November 2019. See [the main page of this report](../README.md) for the other parts.

# Implementing QKD in OpenSSL

## The RIPE quantum Internet hackathon challenge.

The work that we did to add [Quantum Key Distribution (QKD)](https://en.wikipedia.org/wiki/Quantum_key_distribution) support to [OpenSSL](https://www.openssl.org/) was based on the [OpenSSL integration challenge](https://github.com/PEQI19/PEQI-OpenSSL) that was designed by [Wojciech Kozlowski](https://www.linkedin.com/in/wojciech-kozlowski/) for the [Pan-European Quantum Internet Hackathon](https://labs.ripe.net/Members/ulka_athale_1/take-part-in-pan-european-quantum-internet-hackathon) hosted at [QuTech](https://qutech.nl/) at the [Delft University of Technology](https://www.tudelft.nl/) on 5 and 6 November, 2019.

The challenge consists of three parts:

 1. Add QKD support to OpenSSL by invoking the [ETSI KQD API](https://www.etsi.org/deliver/etsi_gs/QKD/001_099/004/01.01.01_60/gs_QKD004v010101p.pdf). Here OpenSSL becomes a consumer of the QKD API. The hackathon organizers had provided a mock implementation of the QKD API for testing purposes.

 2. Implement a specific QKD protocol, namely BB84, on top of the SimulaQron quantum network simulator. Here the BB84 implementation becomes a provider of the QKD API.

The end-goal of the challenge is to use an off-the-shelf browser (e.g. Chrome) and connect it to a secure HTTPS website hosted on an off-the-shelf web server (e.g. Apache), while using the BB84 quantum key distribution algorithm as the key agreement protocol (running a [SimulaQron](http://www.simulaqron.org/) simulated quantum network), instead of the classical Diffie-Hellman protocol that is normally used in classical networks.

During the hackathon, the team that I was a part of worked on part 1 of the challenge. This page describes that work (which was successfully completed). Another team ("team awesome") worked on part 2 of the challenge, which they partially completed. The end-goal (i.e. the integration between part 1 and 2) was not yet achieved. It is my intention to complete the work for part 2, to complete the integration, and to update this document once that work is completed.

## Two approaches to adding QKD support to OpenSSL.

There are two approaches to adding support for QKD in OpenSSL using the ETSI QKD API:
 * Hacking the existing engine-based extension mechanism for Diffie-Hellman.
 * Introducing QKD as a new first-class key exchange protocol.

#### Approach 1: Hacking the existing engine-based extension mechanism for Diffie-Hellman.

TODO

#### Approach 2: Introducing QKD as a new first-class key exchange protocol.

TODO

## Hacking the OpenSSL Diffie-Hellman engine to add QKD.

This section describes in detail how we "hacked" the existing engine-based extension mechanism for Diffie-Hellman to add support for QKD in OpenSSL on top of the ETSI QKD API.

TODO

## The mock implementation of the ETSI QKD API.

This sections describes in details on we created a "mock" (i.e. fake) implementation of the ETSI QKD API that allows us to test OpenSSL QKD without using any quantum network (neither a real quantum network nor a simulated quantum network).

TODO

## How to build and run the code in this repository.

The code in the repository as been tested on Apple macOS and on Ubuntu Linux 18.04 LTS in an Amazon Web Services (AWS) instance.

The instructions for installing, building, and running the code on Ubuntu Linux 18.04 LTS in an AWS instance as as follows:

#### 1. Prepare an AWS Ubuntu instance

1.1. Launch an Ubuntu Linux 18.04 LTS instance in AWS using the default parameters. A t2.tiny instance is sufficient. 

1.2. SSH login to the instance (the default username for AWS Ubuntu instances is "ubuntu").
~~~
ssh -i <your-private-key-file.pem> ubuntu@<ip-address-of-your-aws-instance>
~~~

1.3. Install the build essentials:
~~~~
sudo apt-get update
sudo apt-get install -y build-essential
~~~~

#### 2. Instal OpenSSL

Technically, we don't need to install the OpenSSL source code. The OpenSSL engine mechanism allows us to dynamically load an engine shared library into the OpenSSL binary that is already installed on the instance. Still, we install the OpenSSL source code since reading the source code is essential to understanding how OpenSSL works, and because we will end up changing the OpenSSL source code if and when we introduce QKD as a first-class key exchange protocol at some point in the future.

2.1. Clone OpenSSL from GitHub into the home directory of your AWS instance:
~~~
cd ~
git clone https://github.com/openssl/openssl.git
~~~

2.2. Configure and build the cloned OpenSSL code. Expect the `make` step to take about 5 minutes. Expect the `make test` step to take a bit more than 2 minutes. A few test cases (md2, rc5, gost, etc.) will be skipped, but all other test cases should report "ok". Do **not** run `make install` (we don't want to overwrite the default OpenSSL library).
~~~
cd ~/openssl
./config
make
make test
~~~

#### 3. Instal openssl-qkd

3.1. Close openssl-qkd from GitHub into the home directory of your AWS instance:
~~~
cd ~
git clone https://github.com/brunorijsman/openssl-qkd.git
~~~

3.2. Build the openssl-qkd code:
~~~
cd ~/openssl-qkd
make
~~~

