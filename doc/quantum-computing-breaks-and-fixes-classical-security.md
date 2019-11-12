This is part 2 in a multi-part report describing how we implemented Quantum Key Distribution (QKD) in OpenSSL as part of the pan-European quantum Internet hackathon in Delft on 5 and 6 November 2019. See [the main page of this report](../README.md) for the other parts.

# Quantum computing breaks (and fixes) classical security.

## Diffie-Hellman is vulnerable to quantum attacks.

[Earlier in the document](#the-diffie-hellman-algorithm-details) we described in detail how two communicating end-points can use the Diffie-Hellman algorithm to agree on a shared secret in such a manner that a malicious eavesdropper cannot discover what that shared secret is.

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

There are two problems with the post-quantum cryptography approach:

 1. While these newly proposed post-quantum cryptography algorithms and their associated new quantum-resistant one-way functions are _believed_ to be quantum-safe, they are not _proven_ to be quantum safe. Heck, everyone thought factorization was quantum-safe until Shor shocked the world and proved that it wasn't.

 2. In their current state, the proposed post-quantum cryptography algorithms are so inefficient that they are not (yet) usable in practice: they either very slow and/or they produce gigantic keys.

Despite these problems, it appears that post-quantum cryptography will a more feasible approach to solving quantum-vulnerability of classical security than using Quantum Key Distribution (which is described below). At least for many applications. The reason for this is that post-quantum cryptography does not fundamentally change the architecture of the Internet. Sure, we will have to introduce some new algorithms and probably upgrade some hardware and software. But the architecture does not fundamentally change, and we can continue to use the same types of routers and switches that we are already using and the same fiber that has already been put in the ground.

If we just introduce quantum key distribution on some point-to-point links (possibly with some repeaters in the middle) the architectural impact on the Internet could be limited. But if we want to move to a full-on quantum Internet where quantum routers create entangled Bell pairs between arbitrary end-points anywhere on the planet, we will have to rip and replace the entire Internet and replace it with new quantum router technology that is still very far away on the horizon.

## Quantum Key Distribution (KQD).

TODO

## The BB84 quantum key distribution protocol.

TODO

## QKD in real life.

QKD is probably the most mature application of quantum networking and there already several companies that sell commercially available and deployable QKD devices, including:

 * [ID Quantique](https://www.idquantique.com/quantum-safe-security/products/#quantum_key_distribution)
 * [MagiQ QPN](https://www.magiqtech.com/solutions/network-security/)
