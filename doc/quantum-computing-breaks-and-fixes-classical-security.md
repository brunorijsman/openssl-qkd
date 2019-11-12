This is part 2 in a multi-part report describing how we implemented Quantum Key Distribution (QKD) in OpenSSL as part of the pan-European quantum Internet hackathon in Delft on 5 and 6 November 2019. See [the main page of this report](../README.md) for the other parts.  -- [Bruno Rijsman](https://www.linkedin.com/in/brunorijsman/)

# Quantum computing breaks (and fixes) classical security.

## Diffie-Hellman is vulnerable to quantum attacks.

In [part 1 of this report](#security-in-the-classical-internet) we described in detail how two communicating end-points can use the Diffie-Hellman algorithm to agree on a shared secret in such a manner that a malicious eavesdropper cannot discover what that shared secret is.

This, however, assumes that the malicious eavesdropper only has the power of classical computers at her disposal.

We know that it is easy for classical computers to compute `public_key` when given `g`, `p`, and `private_key`:

<pre>
public_key = (g ^ private_key) mod p
</pre>

However, it is widely believed, but not proven, that it is infeasible for classical computers to compute a discrete (modular) logarithm for large numbers. In other words, given `g`, `p`, and `public_key` (which are all large numbers) it is believed that it is not possible to compute `private_key` by performing the inverse operation (which is a discrete logarithm).

However, in 1994 the American mathematician [Peter Shor](https://en.wikipedia.org/wiki/Peter_Shor) discovered an algorithm, now referred to as [Shor's algorithm](https://en.wikipedia.org/wiki/Shor%27s_algorithm) that allows _quantum_ computers (as opposed to _classical_ computers) to efficiently calculate discrete logarithms for very large numbers.

Thus, at least in theory, the very foundations of security in computer networks have been undermined by the existence of quantum computers and Shor's algorithm. At least in theory, it is possible to break Diffie-Hellman using a quantum computer to discover the private key and the shared secret given g, p, and the public keys.

In practical reality, the Internet is still safe for now. No real quantum computer has ever been built that is sufficiently large and reliable to be able to really execute Shor's algorithm for real-life problems (i.e. for values of g and p that are used in real networks). It it is expected to take many more years before such a quantum computer comes into existence.

On the other hand, some messages that are encrypted today are so sensitive that it is not acceptable that they will become decryptable even years in the future. (And if you put on your tin-foil hat, you could observe that if some secret service already possessed such a quantum computer, they would most certainly not make that information public.)

## Two approaches to repairing network security in the post-quantum era.

There are two different approaches to repairing network security now that it has been discovered to be vulnerable to quantum attacks.
 * Post-quantum cryptography.
 * Quantum Key Distribution (QKD).

We will now discuss each of these approaches.

## Post-quantum cryptography.

The first approach, referred to as [_post-quantum cryptography_](https://en.wikipedia.org/wiki/Post-quantum_cryptography), is based on the observation that only certain very specific parts of classical security have been broken.

Some classical algorithms, notably symmetric encryption such as AES, are not vulnerable to quantum attack. Or, to be more accurate, there is currently no known quantum attack against such algorithms and there is a reasonably strong belief that no quantum attacks will be found.

But any classical algorithm that relies on the difficulty of factoring numbers or computing discrete logarithms is vulnerable to quantum attack. This includes RSA and Diffie-Hellman.

Some problems, such as authentication, digital signatures, and key agreement inherently need one-way mathematical functions: functions that are easy to compute but for which the inverse is difficult to compute. Thus we cannot get rid of one-way functions.

The _post-quantum cryptography_ approach is kind of obvious: let's find new one-way functions that are quantum resistant, i.e. for which even a quantum computer cannot feasibly compute the inverse.

Finding such a new quantum-resistant one-way function is an active area of research. Many post-quantum crypto algorithms have been proposed, including Lattice-based crypto, multivariate crypto, hash-based crypto, code-based crypto, super-singular elliptic curve isogeny crypto, etc. (this list is taken from the [post-quantum cryptography wikipedia page](https://en.wikipedia.org/wiki/Post-quantum_cryptography)). All these new crypto algorithms have in common that they are based on some new one-way function to replace factorization / modular logarithms.

The [Open Quantum Safe organization](https://openquantumsafe.org/) has added support for many of these post-quantum encryption protocols to [OpenSSL](https://www.openssl.org/) by open-sourcing the [Open Quantum-Safe Library (LIBOQS)](https://github.com/open-quantum-safe/liboqs).

There are two problems with the post-quantum cryptography approach:

 1. While these newly proposed post-quantum cryptography algorithms and their associated new quantum-resistant one-way functions are _believed_ to be quantum-safe, they are not _proven_ to be quantum safe. Heck, everyone thought factorization was quantum-safe until Shor shocked the world and proved that it wasn't.

 2. In their current state, the proposed post-quantum cryptography algorithms are so inefficient that they are not (yet) usable in practice: they either very slow and/or they produce gigantic keys.

Despite these problems, it appears that post-quantum cryptography will a more feasible approach to solving quantum-vulnerability of classical security than using Quantum Key Distribution (which is described below). At least for many applications. The reason for this is that post-quantum cryptography does not fundamentally change the architecture of the Internet. Sure, we will have to introduce some new algorithms and probably upgrade some hardware and software. But the architecture does not fundamentally change, and we can continue to use the same types of routers and switches that we are already using and the same fiber that has already been put in the ground.

If we just introduce quantum key distribution on some point-to-point links (possibly with some repeaters in the middle) the architectural impact on the Internet could be limited. But if we want to move to a full-on quantum Internet where quantum routers create entangled Bell pairs between arbitrary end-points anywhere on the planet, we will have to rip and replace the entire Internet and replace it with new quantum router technology that is still very far away on the horizon.

## Quantum Key Distribution (KQD).

The second approach to fixing broken network security is [Quantum Key Distribution (QKD)](https://en.wikipedia.org/wiki/Quantum_key_distribution).

This is a fundamentally different approach to network security that does not rely on one-way functions at all. Instead, QKD relies on the laws of quantum physics to implement security.

It is probably best to look at a specific QKD protocol to understand how it works, and in the next section (where we discuss the BB84 QKD protocol) we will do exactly that.

At the risk of sounding overly vague, let me first point out what the fundamental ideas behind QKD are:

 * Instead of sending classical bits over the network, QKD sends quantum "qubits" over the network. In practice this roughly corresponds to sending light pulses that consist of carefully prepared single photons, so that the quantum effects of the light pulse come into play.

 * There are several specific quantum effects that are relevant to QKD:
   * Quantum "qubits" cannot be copied due to the quantum physics ["no-cloning theorem"](https://en.wikipedia.org/wiki/No-cloning_theorem).
   * In certain circumstances, observing a "qubit" (measuring it) unavoidably causes the value of the qubit to change due to the so-called ["collapse of the quantum wave function"](https://en.wikipedia.org/wiki/Wave_function_collapse).
   * It is possible to create a so-called [Bell pair](https://en.wikipedia.org/wiki/Bell_state) of qubits that can subsequently be used to ["teleport"](https://en.wikipedia.org/wiki/Quantum_teleportation) some arbitrary qubit across the network.

 * QKD protocols cleverly use these quantum effects in the key agreement protocol:
   * In some QKD protocols the two communicating parties use qubits to exchange the bits in the key material, and prepare the qubits in a clever way so that either it can be detected that an adversary observed the key bits.
   * In some QKD protocols, qubits are teleported over the network to prepare the key material.

Once QKD has been used to establish a session key, old-fashioned classical symmetric encryption is used for bulk encryption of the application traffic.

## The BB84 quantum key distribution protocol.

TODO: Document this once I have implemented BB84 on top of SimulaQron (future work).

## QKD in real life.

QKD is probably the most mature application of quantum networking and there already several companies that sell commercially available and deployable QKD devices, including (this is not a complete list):

 * [ID Quantique](https://www.idquantique.com/): [Cerberis and Clavis](https://www.idquantique.com/quantum-safe-security/products/#quantum_key_distribution)
 * [MagiQ](https://www.magiqtech.com): [QPN](https://www.magiqtech.com/solutions/network-security/)
 * [Quantum Xchange](https://quantumxc.com/): [Phio QK](https://quantumxc.com/phio-qk/) and [Phio TX](https://quantumxc.com/phio-tx/)
 * [Quintessence Labs](https://www.quintessencelabs.com): [qOptica](https://www.quintessencelabs.com/products/quantum-key-distribution-qkd/)
 
This list does not include companies that sell post-quantum (classical) cryptography products, but I believe they are included in the Total Addressable Market (TAM) numbers below.

As of 2019 the total marked for QKD devices is around $85 million, and it is estimated to grow to $980 million by 2024 (see [this report](https://www.globenewswire.com/news-release/2019/04/30/1812788/0/en/Inside-Quantum-Technology-Report-says-Quantum-Key-Distribution-Market-to-Reach-980-million-by-2024.html))
