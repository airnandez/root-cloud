## How to install `root-cloud`

Before installing this extension, you need a working installation of ROOT. Please refer to the official documentation for detailed instructions on [how to install ROOT](http://root.cern.ch/drupal/content/downloading-root) on your target platform.

For developing and testing this extension we prefer to install ROOT from sources, so we recommend installing this way. However, if you have installed a binary version you may be able to install this extension if your execution environment matches the one used for building the binary version. You need your target computer to have at least a compatible version of the C++ compiler used for building the binary distribution of ROOT.

You can also install this extension privately and use it with a shared installation of ROOT under someone else's control.

### Installing dependencies
For handling the low level details of the HTTP protocol, this software uses [libNeon](http://www.webdav.org/neon/). To download and install libNeon please do:

```bash
$ cd /tmp
$ curl http://www.webdav.org/neon/neon-0.30.1.tar.gz
$ tar zxvf neon-0.30.1.tar.gz
$ cd neon-0.30.1
$ ./configure --with-ssl
$ make install
```

For building libNeon with support for HTTPS which we highly recommend (and is required by some cloud storage protocols anyway), you need to have OpenSSL installed. It is very likely that you already have it on your system.

As a result of the `make install` command, libNeon components (library, include files, etc.) will be installed under `/usr/local`. If you want to install them in another directory, say `$HOME/libNeon`, use the command below when configuring libNeon:

```bash
$ ./configure --with-ssl --prefix=$HOME/libNeon
```

### Building the extension

Once ROOT and libNeon are installed in your system, you can download and install this extension by following the steps below.

* Go to the directory where ROOT is installed in your system (say `$HOME/ROOT`) and activate that version of ROOT. As a consequence, some environmental variables will be modified (such as `PATH` and `LD_LIBRARY_PATH`) and in particular `$ROOTSYS` will point to the location of your preferred ROOT installation:

```bash
$ source $HOME/ROOT/bin/thisroot.sh    # or source $HOME/ROOT/bin/thisroot.csh if you are using tcsh
```

* Download the sources of this extension by cloning this Git repository into the local directory of your choice, say `$HOME/root-cloud`:

```bash
$ git clone --recursive git://github.com/airnandez/root-cloud $HOME/root-cloud
```

* Compile this extension so that it be compatible with the version of ROOT pointed to by `$ROOTSYS`:

```bash
$ cd $HOME/root-cloud/src
$ make
```

**NOTE:** the provided `Makefile` assumes that the executable `neon-config` (installed by libNeon) is in one directory in your `$PATH`. If that is not your case, do instead:

```bash
$ NEON_CONFIG=/path/to/your/neon-config
$ make
```


### Installing the extension

At this stage you have compiled the extension. Now you have the choice to install it either within the ROOT installation under `$ROOTSYS` or under your `$HOME` for a private use (or in case you are using a shared installation of ROOT that you don't want or you cannot modify).

For installing under `$ROOTSYS`, do

```bash
$ cd $HOME/root-cloud/src && make install
```

These commands install the relevant components of this extension under `$ROOTSYS` as follows:

* the shared object library is copied to `$ROOTSYS/lib/libRootCloudStorage.so`
* the `TFile` plugin is copied to `$ROOTSYS/etc/plugins/TFile/P160_CloudStorage.C`

If instead, you prefer to install this extension under your `$HOME`, do:
```bash
$ cd $HOME/root-cloud/src && make installhome
```

As a result, you now have a directory `HOME/.root` with the hierarchy as shown below:
```bash
$ cd $HOME/.root && tree
.
|-- lib
|   `-- libRootCloudStorage.so
`-- plugins
    `-- TFile
        `-- P160_CloudStorage.C
```

To tell ROOT to look for plugins and shared libraries under your `$HOME/.root` directory, add the following lines to your `$HOME/.rootrc` file:

```
Unix.*.Root.DynamicPath: .:$(HOME)/.root/lib:$(ROOTSYS)/lib:
Unix.*.Root.PluginPath:  .:$(HOME)/.root/plugins:$(ROOTSYS)/etc/plugins:
```

Now you are ready to use this extension.

