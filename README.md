# qt-pods

![Screenshot](https://github.com/cybercatalyst/qt-pods/blob/master/screenshot.png "Screenshot")

qt-pods is an attempt to unify the packaging of fragments of Qt sources.
Inspired by the cocoapods project, each "pod" follows a strict rule of how
its contents are made up, so everyone can easily package her/his code and share
it with other developers.

# How to use

[![Tutorial](http://img.youtube.com/vi/P-gA5g9ZP9I/0.jpg)](http://www.youtube.com/watch?v=P-gA5g9ZP9I)

# pod sources
In order to know which pods are available, you need to specify pod sources.
Pod sources are simply a json-file that list available pods. Currently, only a
single source is hard-coded, but the goal is to let the developer specify his
own sources. Since that source (the "master", or default source) is hosted on
github, developer can easily add their pods by making a pull request:
https://github.com/cybercatalyst/qt-pods-master

Before opening a pull request, you should check whether your submission follows
the pod rules.

# pod rules
Based on my previous experiences with exporting code fragments to be used by
others with Qt, it turned out to be a good idea to share them as static libs
and git submodules. These are the rules that must apply to every pod:

Naming:
* The fragment name is all lower-case and is equal to the repository name.
* The fragment name is unique.
* The distribution of the source within the pod is up to the developer.
* The repository root contains a *.pro file that is a static lib template.
* The *.pro must have exactly have the same name as the repository.
* The repository contains a *.pri file that contains instructions for the final target (for example linker flags of dependencies).
* The resulting library must be in the root of the repository after it has been built.

Formalities:
* The repository must contain a LICENSE file with the license at root.
* The repository must contain a README.md file at root.

# For developers

![Architecture](https://github.com/cybercatalyst/qt-pods/blob/master/architecture.png "Architecture")

Each time you develop an app you will set up a Qt subdirs project.
Each pod you are using will be added as a git submodule, thus keeping track
of the commit-exact version of the pods you are using.

qt-pods will generate a pods.pri next to the subdirs project. By design, all you need
to do is integrating the following statement into your target application's pro file:
```
include(../pods.pri)
```
The pods.pri will set up the include paths, linker flags against pods and even draw in the dependencies of pods via their pris.

Sometimes you want to have submodules that are not pods. In order to allow that, qt-pods manages all subdir targets in a separate file called pods-subdirs.pri.
Usually, if you start a new project from scratch using qt-pods, qt-pods will take care of that and create a subdirs pro with the repository name and include the
following statement:

```
include(pods-subdirs.pri)
```

If you have an existing project, you need to put in that line manually.

# Compile and install
Development takes places with the latest version of Qt. Before submitting a bug report, please check whether that occurs
with the latest version of Qt. If it doesn't, it is not a bug.

Other than that, for a system wide install do this in the build directory:
```
$ sudo make install
```

Run from the command line:
```
$ qt-pods
```

# How does qt-pods compare to inqlude?
At first sight these projects are very similar, but they aren't at all. inqlude's mission is to get an overview of libraries.
Inqlude contains a set of meta-information how a certain library is being installed on a certain system - systemwide.
Inqlude tries to gracefully wrap around everything to get it working somehow - even if it finally needs to compile libraries from source.

In contradiction, qt-pods' scope is the project. You add pods to a projects and qt-pods will set up a code repository that
will keep working even when someone who doesn't use qt-pods wants to build your project. Libraries will be put together at
the time you are cloning a repository, and they will be gone when you don't work on a project anymore. qt-pods expects that pods
are in a certain format, rather than abstracting away differences through a manifest.

# Disclaimer
Please be careful. This software is in early state, use it with caution. Make backups of your repositories.

# Ideas, improvements?

Let me know at jacob@omg-it.works.

