# RDMnet

RDMnet is a library that implements ESTA's upcoming entertainment technology
standard, BSR E1.33. The RDMnet library is maintained by
[ETC](http://www.etcconnect.com).

## Documentation

Documentation for RDMnet is <a href="docs/index.html">here</a>.

## Binary Package

The RDMnet binary package is currently only ported for Microsoft Windows. It
includes the following components:
* *rdmnet_controller_example*: A Qt-based GUI application which does basic
  discovery, display and configuration of RDMnet Components.
* *rdmnet_broker_example*: A console application (which can also be run as a
  Windows service) which implements an RDMnet Broker.
* *rdmnet_device_example*: A console application which implements an RDMnet
  Device.
* *llrp_manager_example*: A console application which implements a basic LLRP
   Manager.

### Download the binary package

Latest: [ ![Download](https://api.bintray.com/packages/etclabs/rdmnet_bin/latest/images/download.svg) ](https://bintray.com/etclabs/rdmnet_bin/latest/_latestVersion)

### Run the binaries

Most of the console applications can be invoked from the command line with no
arguments; however, the Broker currently requires a command line argument:
```
rdmnet_broker_example debug
```

## Note on Qt

The RDMnetControllerGUI application is distributed with binaries for Qt 5.9.7.
This bundling is permitted under the terms of the GNU Lesser General Public
License (LGPL) v.3. For more information on this, see the
[disclaimer](https://github.com/ETCLabs/RDMnet/blob/master/ThirdPartySoftware.txt)
and the <a href="docs/index.html">documentation</a>.
