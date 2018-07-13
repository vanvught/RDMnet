# RDMnet

RDMnet is a library that implements ESTA's upcoming entertainment technology
standard, BSR E1.33. The RDMnet library is maintained by
[ETC](http://www.etcconnect.com).

## Documentation

Documentation for RDMnet is <a href="docs/index.html">here</a>.

## Binary Package

The RDMnet binary package includes the following components:
* *RDMnetControllerGUI*: A Qt-based GUI application which does basic discovery,
  display and configuration of RDMnet Components.
* *broker*: A console application (which can also be run as a Windows service)
  which implements an RDMnet Broker.
* *device*: A console application which implements an RDMnet Device.
* *fakeway*: A console application which implements an RDMnet Gateway, using
  one or more ETC [Gadget II](https://www.etcconnect.com/Products/Networking/Gadget-II/Features.aspx)
  devices to communicate with RDM fixtures.
* *manager*: A console application which implements a basic LLRP Manager.

Because of the early development state of the library and some licensing
restrictions imposed by third-party software, there are a few steps necessary
to run the RDMnet binaries.

1. Download and extract the [binary package](https://github.com/ETCLabs/RDMnet/releases/download/v0.1.0.2/rdmnet_bin_package.zip).
2. Install the [Microsoft Visual C++ 2015 Redistributable](https://www.microsoft.com/en-us/download/details.aspx?id=52685).
   To correctly run all of the apps, the 32- and 64-bit versions must both be installed.
3. Install Apple's [Bonjour Print Services for Windows](https://support.apple.com/kb/dl999?locale=en_US).
4. Run the applications.

Most of the console applications can be invoked from the command line with no
arguments; however, the Broker currently requires a command line argument:
```
broker.exe -debug
```

If you are using the fakeway application with a Gadget II, USB drivers are
required. These drivers are installed with any installation of ETC's
[Eos](https://www.etcconnect.com/Products/Consoles/Eos-Family/)
or [Concert](https://www.etcconnect.com/Products/Networking/Software/Software.aspx)
software.

## Note on Qt

The RDMnetControllerGUI application is distributed with binaries for Qt 5.9.1.
This bundling is permitted under the terms of the GNU Lesser General Public
License (LGPL) v.3. For more information on this, see the
[disclaimer](https://github.com/ETCLabs/RDMnet/blob/master/ThirdPartySoftware.txt)
and the <a href="docs/index.html">documentation</a>.
