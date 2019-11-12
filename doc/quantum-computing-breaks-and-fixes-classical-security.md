# Diffie-Hellman Can Be Broken By Quantum Computers

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


# Two Approaches To Fixing Key Agreement In The Post-Quantum Era

TODO

# The BB84 Quantum Key Distribution Protocol

TODO

