# GNSE Software
The software provides a boilerplate of applications and libraries that can be used as it is or customized to serve a specific use case.

![GNSE Software Architecture](/Docs/img_sw/sw_arch.png)

## Structure

- [target](./target/README.md) folder contains STM32WL low level target files
- [lib](./lib/README.md) folder contains SW libraries used by the various applications
- [app](./app/README.md) folder contains SW applications

## Documentation

The [Generic Node documentation website](https://www.genericnode.com/docs/) provides information about the software features and how to [get started with the software development](https://www.genericnode.com/docs/getting-started/se-sw/).

Users can also generate a readable [Doxygen](https://www.doxygen.nl/index.html) html file by invoking the following shell command:

```
$ doxygen Docs/doxygen/Doxyfile
```

The generated html file located at `Docs/doxygen/doxygen_out/html/index.html` will contain comprehensive documentation of the source code and the system APIs.

## Legal

Copyright © 2021 The Things Industries B.V.

[Contact The Things Industries](https://thethingsindustries.com/contact/) for licensing.
