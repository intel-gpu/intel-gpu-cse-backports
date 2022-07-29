# Intel® Converged Security Engine Backports

This repo holds backports of the cse driver.

The driver is provided as a DKMS module targeting distinct
versions of various distros. Each backport is on a topic 
branch. Current support status is documented on the topic branch.

## Dependencies
This driver is part of a collection of kernel mode drivers 
that together enable support for Intel graphics. The backports 
collection within https://github.com/intel-gpu includes:

  - [i915](https://github.com/intel-gpu/intel-gpu-i915-backports) - The main graphics driver (includes a compatible DRM subsystem and dmabuf if necessary).
  - [cse](https://github.com/intel-gpu/intel-gpu-cse-backports) - Converged Security Engine
  - [pmt](https://github.com/intel-gpu/intel-gpu-pmt-backports) - Intel Platform Telemetry
  - [firmware](https://github.com/intel-gpu/intel-gpu-firmware) - Contains firmware required by i915.

Each project is tagged consistently so when pulling these repos pull the same tag.

## How to generate the DKMS module

### Ubuntu
#### Install dependencies:

NOTE: replace linux-headers with headers specific to your kernel
as needed.

```
sudo apt install \
    dkms \
    make \
    linux-headers-5.14.0-1034-oem
```

#### Create debian package:
```
BUILD_VERSION=1 make -f Makefile.dkms dkmsdeb-pkg
```

#### Install dkms deb package:

```
sudo dpkg -i intel-platform-cse-dkms-*.deb
```

### SLES 

#### Install dependencies:

```
sudo zypper install \
   dkms \
   make \
   linux-glibc-devel \
   lsb-release \
   rpm-build
```

#### Build and install dkms package
```
BUILD_VERSION=1 make -f Makefile.dkms dkmsrpm-pkg
```

The resulting rpm package will be located in:
/usr/src/packages/RPMS/x86_64/

Install with:

```
sudo zypper install --allow-unsigned-rpm \
    /usr/src/packages/RPMS/x86_64/intel-platform-cse-dkms-*.rpm
```

### RHEL

#### Install dependencies:

Install packages

```
sudo dnf install \
   git \
   kernel-headers \
   make \
   rpm-build
```

Install dkms subsystem support:

```
git clone https://github.com/dell/dkms dkms
cd dkms
make install-redhat
```

#### Build and install dkms package
```
BUILD_VERSION=1 make -f Makefile.dkms dkmsrpm-pkg
```

The rpm package will be placed in $HOME/rpmbuild/RPMS/x86_64.
For example:

```
/home/user/rpmbuild/RPMS/x86_64/intel-platform-cse-dkms-2022.7-1.x86_64.rpm
```

To install, run:

```
cp $HOME/rpmbuild/RPMS/x86_64/*.rpm .
sudo dnf install intel-platform-cse-dkms*.rpm
```
