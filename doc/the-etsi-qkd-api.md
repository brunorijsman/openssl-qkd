This is part 3 in a multi-part report describing how we implemented Quantum Key Distribution (QKD) in OpenSSL as part of the pan-European quantum Internet hackathon in Delft on 5 and 6 November 2019. See [the main page of this report](../README.md) for the other parts.

# The ETSI QKD API.

The roll-of-the-tongue acronym _ETSI QKD API_ stands for the [European Telecommunications Standards Organization (ETSI)](https://www.etsi.org/) [Quantum Key Distribution (QKD)](https://en.wikipedia.org/wiki/Quantum_key_distribution) Application Programming Interface (API).

In [part 2 of this report](#quantum-computing-breaks-and-fixes-classical-security) we described what is broken in classical security, how Quantum Key Distribution (QKD) can fix it, what some of the theory behind QKD is, and how there are already some commercial companies that offer QKD devices for sale.

For now, the commercially available QKD devices are rather large stand-alone devices that are not yet integrated into routers or switches or end-point computers on the network. Thus the application that wants to consume QKD sits on a different device than the stand-alone device that implements QKD.

As a result, we need some sort of interface, a so-called Application Programming Interface (API) between QKD consumer (the application) and the QKD provider (the stand-alone QKD device).

The [European Telecommunications Standards Organization (ETSI)](https://www.etsi.org/) has defined exactly such an API, namely [ETSI GS QKD 004 V1.1.1 (2010-12)](https://www.etsi.org/deliver/etsi_gs/QKD/001_099/004/01.01.01_60/gs_QKD004v010101p.pdf): Quantum Key Distribution (QKD); Application Interface. In [part 4 of this report](#implementing-qkd-in-openssl.md) we will use this API to add QKD support to [OpenSSL](https://www.openssl.org/).

In fact, there is an [Industry Specification Group (ISG) in ETSI that defines standards for Quantum Key Distribution for Users](https://www.etsi.org/committee/qkd) that has produced multiple standards in the area of QKD:

 * [ETSI GS QKD 002 V1.1.1 (2010-06)](https://www.etsi.org/deliver/etsi_gs/qkd/001_099/002/01.01.01_60/gs_qkd002v010101p.pdf): Quantum Key Distribution (QKD); Use Cases.

 * [ETSI GR QKD 003 V2.1.1 (2018-03)](https://www.etsi.org/deliver/etsi_gr/QKD/001_099/003/02.01.01_60/gr_QKD003v020101p.pdf): Quantum Key Distribution (QKD); Components and Internal Interfaces.

 * [ETSI GS QKD 004 V1.1.1 (2010-12)](https://www.etsi.org/deliver/etsi_gs/QKD/001_099/004/01.01.01_60/gs_QKD004v010101p.pdf): Quantum Key Distribution (QKD); Application Interface.

 * [ETSI GS QKD 005 V1.1.1 (2010-12)](https://www.etsi.org/deliver/etsi_gs/QKD/001_099/005/01.01.01_60/gs_QKD005v010101p.pdf): Quantum Key Distribution (QKD); Security Proofs.

 * [ETSI GR QKD 007 V1.1.1 (2018-12)](https://www.etsi.org/deliver/etsi_gr/QKD/001_099/007/01.01.01_60/gr_QKD007v010101p.pdf): Quantum Key Distribution (QKD); Vocabulary.

 * [ETSI GS QKD 008 V1.1.1 (2010-12)](https://www.etsi.org/deliver/etsi_gs/QKD/001_099/008/01.01.01_60/gs_QKD008v010101p.pdf): Quantum Key Distribution (QKD); QKD Module Security Specification.

 * [ETSI GS QKD 011 V1.1.1 (2016-05)](https://www.etsi.org/deliver/etsi_gs/QKD/001_099/011/01.01.01_60/gs_QKD011v010101p.pdf): Quantum Key Distribution (QKD); Component characterization: characterizing optical components for QKD systems.

 * [ETSI GS QKD 012 V1.1.1 (2019-02)](https://www.etsi.org/deliver/etsi_gs/QKD/001_099/012/01.01.01_60/gs_QKD012v010101p.pdf): Quantum Key Distribution (QKD); Device and Communication Channel Parameters for QKD Deployment.

 * [ETSI GS QKD 014 V1.1.1 (2019-02)](https://www.etsi.org/deliver/etsi_gs/QKD/001_099/014/01.01.01_60/gs_QKD014v010101p.pdf): Quantum Key Distribution (QKD); Protocol and data format of REST-based key delivery API.

 ETSI also published an interesting report that describes practical attacks and counter measures on the QKD implementations: [ETSI White Paper No. 27: Implementation Security of Quantum Cryptography; Introduction, challenges, solutions](https://www.etsi.org/images/files/ETSIWhitePapers/etsi_wp27_qkd_imp_sec_FINAL.pdf).

 Note that these are vulnerabilities due the the _implementation_ of the QKD system as opposed to vulnerabilities due to flaws in the QKD theory itself.







