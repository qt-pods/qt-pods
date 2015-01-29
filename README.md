# qt-pods

![Screenshot](https://github.com/cybercatalyst/qt-pods/blob/master/screenshot.png "Screenshot")

qt-pods is an attempt to unify the packaging of fragments of Qt sources.
Inspired by the cocoapods project, each fragment follows a strict rule of how
its contents are made up, so everyone can easily package her/his code and share
it with other developers.

Based on my previous experiences with exporting code fragments to be used by
others with Qt, it turned out to be a good idea to share them as static libs
and git submodules. These are the rules that must apply to every fragment:

Naming:
* The fragment name is all lower-case and is equal to the repository name.
* The fragment name starts with a "q".
* The fragment name is unique.

Folder structure:
* The distribution of the source is up to the developer.
* The repository root contains a *.pro file that is a lib template.
* The lib is configured as staticlib.
* The *.pro must have exactly have the same name as the repository.
* The repository contains a *.pri file that contains instructions for the final target (for example linker flags of dependencies).
* The resulting library must be in the root of the repository after it has been built.

Formalities:
* The repository must contain a LICENSE file with the license at root.
* The repository must contain a README.md file with the name of the author at root.

I have written myself a script that can draw in fragments that follow the above
rules, but it is not very flexible. qt-pods is a seconds attempt to have a nice
tool for that job.

# How does this work then?

Basically, each time you develop an app you will set up a Qt subdirs project.
Each fragment you are using will be added as a git submodule, thus keeping track
of the commit-exact version of the fragments you are using. That means, that on
one hand, you will never run into version problems (you can happily keep an
older version running) and you still can use all the benefits from a git
submodule (such as bisecting to find out when a newer version of a fragments
breaks your app). Even better, if you encounter a bug in foreign code, you can
fork it on github, correct it yourself and open a pull request. How cool is that?

That means, that by using git submodules, you are not only on the "consumer" side
of packages, using what others have packaged, you are also able to actively modify
these as you have a ready-to-go development copy of each fragment. I have used this
so much to constantly develop outsourced code and improve without the need of 
repackaging over and over again. Finally, by making this step that easy, you tend
to often improve your outsourced in small, incremental steps, rather than designing
castles in the air.

# Ideas, improvements?

Let me know at jacob@omg-it.works.

